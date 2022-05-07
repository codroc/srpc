#ifndef SRPC_NET_POLL_H
#define SRPC_NET_POLL_H


#include <sys/epoll.h>
#include <vector>

#define MAX_EVENTS 20
class Channel;
class Epoll {
public:
    Epoll();
    ~Epoll();
    void updata(void* data, uint32_t events);
    void enroll(void* data, uint32_t events);
    void unenroll(void* data, uint32_t events);

    // 轮询一次
    void poll(std::vector<Channel*>& vec, int timeout = -1);
private:
    int _epfd{};
    struct epoll_event _evs[MAX_EVENTS];
};

#endif
