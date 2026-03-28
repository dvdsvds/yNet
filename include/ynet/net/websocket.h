#pragma once

#include "ynet/net/connection.h"
#include "ynet/core/request.h"
#include <functional>
namespace ynet {
    class WebSocket {
        private:
            Connection* conn;
            std::function<void()> on_open;
            std::function<void(const std::string&)> on_message;
            std::function<void()> on_close;
        public:
            WebSocket(Connection* conn) : conn(conn) {};
            static bool isUpgrade(const Request& req);
            static std::string handshake(const Request& req);
            std::string readFrame();
            void sendFrame(const std::string& data, uint8_t opcode);
            void onOpen(std::function<void()> cb) { on_open = cb; }
            void onMessage(std::function<void(const std::string&)> cb) { on_message = cb; }
            void onClose(std::function<void()> cb) { on_close = cb; }
            void run();
    };
}
