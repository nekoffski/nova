#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include <kc/core/Singleton.hpp>

#include "starlight/core/Core.hh"

namespace sl {

class TaskQueue : public kc::core::Singleton<TaskQueue> {
    using Queue = std::vector<std::function<void()>>;

public:
    enum class Type {
        preFrameRender,
        postFrameRender,
        preFrameUpdate,
        postFrameUpdate
    };

    template <typename C>
    requires Callable<C>
    void push(Type type, C&& callback) {
        m_queues[type].emplace_back(std::move(callback));
    }

    void dispatchQueue(Type type);

private:
    std::unordered_map<Type, Queue> m_queues;
};

}  // namespace sl
