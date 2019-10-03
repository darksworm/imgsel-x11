#pragma once

#include <mutex>
#include <queue>

template<class T>
class ThreadSafeQueue {
    std::queue<T> q;
    std::mutex m;

public:
    ThreadSafeQueue() = default;

    void push(T elem) {
        std::lock_guard<std::mutex> lock(m);
        q.push(elem);
    }

    bool next(T &elem) {
        std::lock_guard<std::mutex> lock(m);
        if (q.empty()) {
            return false;
        }

        elem = q.front();
        q.pop();
        return true;
    }
};


