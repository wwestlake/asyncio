//
// Created by netcan on 2021/10/09.
//

#ifndef ASYNCIO_EPOLL_SELECTOR_H
#define ASYNCIO_EPOLL_SELECTOR_H
#include <asyncio/asyncio_ns.h>
#include <asyncio/selector/event.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <vector>
#include <fmt/core.h>
ASYNCIO_NS_BEGIN
struct EpollSelector {
    EpollSelector(): epfd_(epoll_create1(0)) {
        if (epfd_ < 0) {
            perror("epoll_create1");
            throw;
        }
    }
    std::vector<Event> select(int timeout /* ms */) {
        errno = 0;
        std::vector<epoll_event> events;
        events.resize(register_event_count_);
        int ndfs = epoll_wait(epfd_, events.data(), register_event_count_, timeout);
        std::vector<Event> result;
        for (size_t i = 0; i < ndfs; ++i) {
            result.emplace_back(Event {
                .data = events[i].data.ptr
            });
        }
        return result;
    }
    ~EpollSelector() {
        if (epfd_ > 0) { close(epfd_); }
    }
    bool is_stop() { return register_event_count_ == 1; }
    void register_event(const Event& event) {
        epoll_event ev{ .events = event.events, .data {.ptr = event.data } };
        if (epoll_ctl(epfd_, EPOLL_CTL_ADD, event.fd, &ev) == 0) {
            ++register_event_count_;
        }
    }

    void remove_event(const Event& event) {
        epoll_event ev{ .events = event.events };
        if (epoll_ctl(epfd_, EPOLL_CTL_DEL, event.fd, &ev) == 0) {
            --register_event_count_;
        }
    }
private:
    int epfd_;
    int register_event_count_ {1};
};
ASYNCIO_NS_END
#endif // ASYNCIO_EPOLL_SELECTOR_H
