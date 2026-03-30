#include <exception>
#include <openssl/ssl.h>
#include <string>
#include <iostream>
#include "ynet/core/server.h"

using namespace ynet;

void Server::handleWebSocket(Connection& conn, Request& req) {
    auto it = ws_routes.find(req.getPath());
    if(it != ws_routes.end()) {
        std::string res = WebSocket::handshake(req);
        conn.write(res.c_str(), res.size());
        WebSocket ws(&conn);
        it->second(ws);
    }
}

void Server::handleRequest(int fd, Connection& conn) {
    if(conn.getSSL() && !conn.handshake()) {
        std::lock_guard<std::mutex> lock(conn_mtx);
        connections.erase(fd);
        return;
    }

    std::string raw;
    char buf[4096];

    while(true) {
        ssize_t n = conn.read(buf, sizeof(buf));
        if(n <= 0) break;
        raw.append(buf, n);
        if(raw.find("\r\n\r\n") != std::string::npos) break;
    }

    size_t header_end = raw.find("\r\n\r\n");
    if(header_end != std::string::npos) {
        size_t body_start = header_end + 4;
        size_t content_length = 0;
        size_t cl_pos = raw.find("Content-Length: ");
        if(cl_pos != std::string::npos) {
            size_t cl_end = raw.find("\r\n", cl_pos);
            content_length = std::stoul(raw.substr(cl_pos + 16, cl_end - cl_pos - 16));
        }
        if(content_length > config.max_upload_size) {
            Response res = Response::error(413, "Upload size exceeds limit");
            res.send(conn);
            if(conn.getSSL()) SSL_shutdown(conn.getSSL());
            {
                std::lock_guard<std::mutex> lock(conn_mtx);
                connections.erase(fd);
            }
            return;
        }
        while(raw.size() - body_start < content_length) {
            ssize_t n = conn.read(buf, sizeof(buf));
            if(n <= 0) break;
            raw.append(buf, n);
        }
    }

    if(raw.empty()) {
        std::lock_guard<std::mutex> lock(conn_mtx);
        connections.erase(fd);
        return;
    }

    auto parseResult = Request::parse(raw.c_str(), raw.size());
    parseResult.setClientIP(conn.getClientIP());
    if(parseResult.isParseError()) {
        Response res = Response::error(400);
        res.send(conn);
        {
            std::lock_guard<std::mutex> lock(conn_mtx);
            connections.erase(fd);
        }
        return;
    }

    if(WebSocket::isUpgrade(parseResult)) {
        handleWebSocket(conn, parseResult);
        std::lock_guard<std::mutex> lock(conn_mtx);
        connections.erase(fd);
        return;
    }

    auto handler = router->resolve(parseResult.getMethod(), parseResult.getPath(), parseResult);
    Response res;

    std::function<void()> next = [&](){
        if(handler) {
            (*handler)(parseResult, res);
        } else {
            bool served = false;
            for(auto& static_file : static_files) {
                if(static_file.tryServe(parseResult, res)) {
                    served = true;
                    break;
                }
            }
            if(!served) {
                if(error_handlers.find(404) != error_handlers.end()) {
                    res = error_handlers[404](parseResult);
                } else {
                    res = Response::error(404);
                }
            }
        }
    };

    for(int i = middlewares.size() - 1; i >= 0; i--) {
        next = [&, i, next]() {
            middlewares[i](parseResult, res, next);
        };
    }

    try {
        next();
    } catch(std::exception& error) {
        if(error_handlers.find(500) != error_handlers.end()) {
            res = error_handlers[500](parseResult);
        } else {
            res = Response::error(500);
        }
    }
    res.send(conn);

    auto conn_header = parseResult.getHeader("Connection");
    if(conn_header.has_value() && conn_header.value() == "close") {
        std::lock_guard<std::mutex> lock(conn_mtx);
        connections.erase(fd);
    } else {
        ev.add(fd, EPOLLIN);
    }
}

void Server::start() {
    if(tcp_listener.bind() == -1) {
        std::cerr << "bind failed" << std::endl;
        return ;
    }
    if(tcp_listener.listen() == -1) {
        std::cerr << "listen failed" << std::endl;
        return ;
    }

    std::cerr << "[ynet] listening on " << config.bind_addr << ":" << config.port << (config.use_tls ? " (HTTPS)" : " (HTTP)") << std::endl;
    ev.add(tcp_listener.getFd(), EPOLLIN);
    std::vector<epoll_event> events(config.max_connections);

    while(1) {
        int wait = ev.wait(events.data(), -1);
        for(int i = 0; i < wait; i++) {
            if(events[i].data.fd == tcp_listener.getFd()) {
                auto conn = tcp_listener.accept();
                if(!conn) continue;
                int fd = conn->getClientFd();
                ev.add(fd, EPOLLIN);
                if(tls_ctx) {
                    SSL* ssl = tls_ctx->wrap(conn->getClientFd());
                    conn->setSSL(ssl);
                }

                {
                    std::lock_guard<std::mutex> lock(conn_mtx);
                    connections[fd] = std::move(conn);
                }

            } else {
                int fd = events[i].data.fd;
                ev.remove(fd);
                tp.submit([this, fd]() {
                    Connection* conn; 
                    {
                        std::lock_guard<std::mutex> lock(conn_mtx);
                        conn = connections[fd].get();
                    }
                    handleRequest(fd, *conn);
                });
            }
        }
    }
}

