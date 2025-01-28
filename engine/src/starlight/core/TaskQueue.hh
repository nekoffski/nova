#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "Singleton.hh"
#include "Core.hh"
#include "Concepts.hh"

namespace sl {

class TaskQueue : public Singleton<TaskQueue> {
    using Queue = std::vector<std::function<void()>>;

public:
    enum class Type { preFrame, postFrame };

    template <typename C>
    requires Callable<C>
    void push(Type type, C&& callback) {
        m_queues[type].emplace_back(std::move(callback));
    }

    template <typename C>
    requires Callable<C>
    void callPostFrame(C&& callback) {
        push(Type::postFrame, std::move(callback));
    }

    template <typename C>
    requires Callable<C>
    void callPreFrame(C&& callback) {
        push(Type::preFrame, std::move(callback));
    }

    void dispatchQueue(Type type);

private:
    std::unordered_map<Type, Queue> m_queues;
};

}  // namespace sl
