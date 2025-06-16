#include "App/IdPool.h"

int IdPool::acquire() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_freeIds.empty()) {
        int id = m_freeIds.front();
        m_freeIds.pop();
        return id;
    }

    return m_nextId++;
}


void IdPool::release(int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_freeIds.push(id);
}


void IdPool::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_freeIds = std::queue<int>();
    m_nextId = 0;
}