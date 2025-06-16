#pragma once
#include <queue>
#include <mutex>

class IdPool {
public:
    int acquire();
    void release(int id);

    void clear();

private:
    std::mutex m_mutex;

    std::queue<int> m_freeIds;
    int m_nextId = 0;
};