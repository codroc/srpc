#include "poll.h"
#include "channel.h"
#include "flog.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

Epoll::Epoll() 
    : _epfd(::epoll_create(1))
{}

Epoll::~Epoll() {
    ::close(_epfd);
}

void Epoll::updata(void* data, uint32_t events) {
    struct epoll_event ev{};
    ev.data.ptr = data;
    ev.events = events;
    int fd = reinterpret_cast<Channel*>(data)->get_fd().fd();
    if (-1 == ::epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev)) {
        LOG_ERROR << "Epoll::updata error! " << ::strerror(errno) << "\n"; 
        ::exit(-1);
    }
}

void Epoll::enroll(void* data, uint32_t events) {
    struct epoll_event ev{};
    ev.data.ptr = data;
    ev.events = events;
    int fd = reinterpret_cast<Channel*>(data)->get_fd().fd();
    if (-1 == ::epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev)) {
        LOG_ERROR << "Epoll::enroll error! " << ::strerror(errno) << "\n"; 
        ::exit(-1);
    }
}

void Epoll::unenroll(void* data, uint32_t events) {
    int fd = reinterpret_cast<Channel*>(data)->get_fd().fd();
    if (-1 == ::epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL)) {
        LOG_ERROR << "Epoll::unenroll error! " << ::strerror(errno) << "\n"; 
        ::exit(-1);
    }
}

void Epoll::poll(std::vector<Channel*>& vec, int timeout) {
    int ret = ::epoll_wait(_epfd, _evs, MAX_EVENTS, timeout);
    if (ret == -1) {
        LOG_ERROR << "Epoll::poll epoll_wait error: "
                  << ::strerror(errno) << "\n";
        ::exit(-1);
    } else {
        for (int i = 0;i < ret; ++i) {
            Channel* p = static_cast<Channel*>(_evs[i].data.ptr);    
            p->set_revents(_evs[i].events);
            vec.push_back(p);
        }
    }
}
