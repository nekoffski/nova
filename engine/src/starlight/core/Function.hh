#pragma once

#include <utility>
#include <concepts>

#include "Concepts.hh"

namespace sl {

struct SingleCaller {
    template <typename Callback, typename... Args>
    requires Callable<Callback, void, Args...>
    SingleCaller(Callback&& callback, Args&&... args) {
        callback(std::forward<Args>(args)...);
    }
};

template <typename F>
requires std::is_invocable_v<F>
class LazyEvaluator {
    using Result = decltype(std::declval<const F>()());

public:
    explicit LazyEvaluator(F&& evaluable) : m_evaluable(std::move(evaluable)) {}

    constexpr operator Result() const { return m_evaluable(); }

    template <typename T>
    requires(std::is_constructible_v<T, Result>)
    constexpr operator T() const {
        return T{ m_evaluable() };
    }

private:
    F m_evaluable;
};

template <typename F> constexpr auto lazyEvaluate(F&& f) {
    return LazyEvaluator{ std::forward<F>(f) };
}

}  // namespace sl
