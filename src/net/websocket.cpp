#include <iostream>
#include <sys/socket.h>
#include "ynet/util/crypto.h"
#include <ynet/net/websocket.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace ynet;

bool WebSocket::isUpgrade(const Request& req) {
    auto upgrade = req.getHeader("Upgrade");
    auto conn = req.getHeader("Connection");
    if((upgrade.has_value() && upgrade.value() == "websocket") && 
       (conn.has_value() && conn.value().find("Upgrade") != std::string::npos)) {
        return true;
    }
    return false;
}

std::string WebSocket::handshake(const Request &req) {
    std::string key = req.getHeader("Sec-WebSocket-Key").value();
    std::string combined = key + "258EAFA5-E914-47DA-95CA-5AB9DC176B71";
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.size(), hash);
    std::string accept = base64_encode(hash, SHA_DIGEST_LENGTH);

    return "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: " + accept + "\r\nSec-WebSocket-Version: 13\r\n\r\n";
}

std::string WebSocket::readFrame() {
    char header[2];
    conn->read(header, 2);
    // ssize_t r = conn->read(header, 2);

    uint8_t opcode = header[0] & 0x0F;
    bool masked = header[1] & 0x80;
    uint64_t payload_len = header[1] & 0x7F;

    if(payload_len == 126) {
        char ext[2];
        conn->read(ext, 2);
        payload_len = (static_cast<uint8_t>(ext[0]) << 8) | static_cast<uint8_t>(ext[1]);
    } else if(payload_len == 127) {
        char ext[8];
        conn->read(ext, 8);
        payload_len = 0;
        for(int i = 0; i < 8; i++) {
            payload_len = (payload_len << 8) | static_cast<uint8_t>(ext[i]);
        }
    }

    char mask_key[4] = {0};
    if(masked == 1) {
        conn->read(mask_key, 4);
    }
    std::string payload(payload_len, 0);
    conn->read(&payload[0], payload_len);

    if(masked) {
        for(size_t i = 0; i < payload_len; i++) {
            payload[i] ^= mask_key[i % 4];
        }
    }

    if(opcode == 0x8) {
        conn->close();
        return "";
    } else if(opcode == 0x9) {
        sendFrame(payload, 0xA);
    }

    return payload;
}

void WebSocket::sendFrame(const std::string& data, uint8_t opcode) {
    std::string frame;
    frame += static_cast<char>(0x80 | opcode);

    if(data.size() <= 125) {
        frame += static_cast<char>(data.size());
    } else if(data.size() <= 65535) {
        frame += static_cast<char>(126);
        frame += static_cast<char>((data.size() >> 8) & 0xFF);
        frame += static_cast<char>(data.size() & 0xFF);
    } else {
        frame += static_cast<char>(127);
        for(int i = 7; i >= 0; i--) {
            frame += static_cast<char>((data.size() >> (i * 8)) & 0xFF);
        }
    }

    frame += data;
    conn->write(frame.c_str(), frame.size());
}


// void WebSocket::run() {
//     on_open();
//     while(1) {
//         std::string payload = readFrame();
//         if(payload.empty()) break;
//         on_message(payload);
//     }
//     on_close();
// }
void WebSocket::run() {
    int epfd = epoll_create1(0);
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = conn->getClientFd();
    epoll_ctl(epfd, EPOLL_CTL_ADD, conn->getClientFd(), &ev);
    
    on_open();
    
    struct epoll_event events[1];
    while(true) {
        int n = epoll_wait(epfd, events, 1, -1);
        if(n <= 0) continue;
        std::string payload = readFrame();
        if(payload.empty()) break;
        on_message(payload);
    }
    on_close();
    close(epfd);
}
