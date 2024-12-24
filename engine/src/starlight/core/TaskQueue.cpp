#include "TaskQueue.hh"

namespace sl {

void TaskQueue::dispatchQueue(Type type) {
    auto& queue = m_queues[type];
    for (auto& task : queue) task();
    queue.clear();
}

}  // namespace sl
