#include <ynet/server.h>
#include <iostream>

using namespace ynet;

void Server::start() {
    if(tcp_listener.bind() == -1) {
        std::cerr << "bind failed" << std::endl;
        return ;
    }
    if(tcp_listener.listen() == -1) {
        std::cerr << "listen failed" << std::endl;
        return ;
    }

    ev.add(tcp_listener.getFd(), EPOLLIN);
    struct epoll_event events[config.max_connections];

    while(1) {
        int wait = ev.wait(events, -1);
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
                    char buf[4096];
                    ssize_t n = conn->read(buf, sizeof(buf));

                    if(n <= 0) {
                        {   
                            std::lock_guard<std::mutex> lock(conn_mtx);
                            connections.erase(fd);
                        }
                        return ;
                    };

                    auto parseResult = Request::parse(buf, n);
                    parseResult.setClientIP(conn->getClientIP());
                    if(WebSocket::isUpgrade(parseResult)) {
                        auto it = ws_routes.find(parseResult.getPath());
                        if(it != ws_routes.end()) {
                            std::string res = WebSocket::handshake(parseResult);
                            conn->write(res.c_str(), res.size());
                            WebSocket ws(conn);
                            it->second(ws);
                        }
                        std::lock_guard<std::mutex> lock(conn_mtx);
                        connections.erase(fd);
                        return ;
                    }

                    auto handler = router.resolve(parseResult.getMethod(), parseResult.getPath());
                    Response res;

                    std::function<void()> next = [&](){
                        if(handler) {
                            (*handler)(parseResult, res);
                        } else {
                            res.status(404).body("Not Found");
                        }
                    };

                    for(int i = middlewares.size() - 1; i >= 0; i--) {
                        next = [&, i, next]() {
                            middlewares[i](parseResult, res, next);
                        };
                    }

                    next();
                    res.send(*conn);
                    {
                        std::lock_guard<std::mutex> lock(conn_mtx);
                        connections.erase(fd);
                    }
                });
            }
        }
    }
}

void Server::stop() {
    tcp_listener.close();
}

void Server::mount(Router& r) {
    router = r;
}

void Server::use(Middleware mw) {
    middlewares.push_back(mw);
}

void Server::ws(const std::string& path, WsHandler handler) {
    ws_routes[path] = handler;
}
