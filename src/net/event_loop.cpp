#include "ynet/net/event_loop.h"
#include <unistd.h>
#include <fcntl.h>

using namespace ynet;

EventLoop::~EventLoop() {
    if(epfd != -1) close(epfd);
}

int EventLoop::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL); 
    if(flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int EventLoop::add(int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
}

int EventLoop::remove(int fd) {
    return epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
}

int EventLoop::wait(struct epoll_event* events, int timeout) {
    return epoll_wait(epfd, events, max_events, timeout);
}
