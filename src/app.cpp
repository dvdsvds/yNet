#include <ynet/app.h>

using namespace ynet;

ErrorRoute App::onError(int code) {
    return ErrorRoute(server, code);
}

void App::listen() {
    server.mount(router);
    server.start();
}
