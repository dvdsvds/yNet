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

    while(1) {
        auto conn = tcp_listener.accept();
        if(!conn) continue;

        if(tls_ctx) {
            SSL* ssl = tls_ctx->wrap(conn->getClientFd());
            conn->setSSL(ssl);
        }

        char buf[4096];
        ssize_t n = conn->read(buf, sizeof(buf));
        if(n <= 0) continue;

        auto parseResult = Request::parse(buf, n);
        parseResult.setClientIP(conn->getClientIP());
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
