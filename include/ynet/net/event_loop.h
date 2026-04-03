#pragma once

#include <cstdint>
#include <stdexcept>
#include <sys/epoll.h>

namespace ynet {
    class EventLoop {
        private:
            int epfd;
            int max_events;
        public:
            EventLoop(int max_events) : epfd(epoll_create1(0)), max_events(max_events) {
                if(epfd == -1) throw std::runtime_error("epoll_create1 failed");
            }
            ~EventLoop();
            static int setNonBlocking(int fd);
            int add(int fd, uint32_t events);
            int remove(int fd);
            
            int wait(struct epoll_event* events, int timeout);
    };
}
