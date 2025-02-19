#pragma once

#include <vector>
#include <unordered_map>
#include <utility>
#include <ranges>

#include "starlight/core/Core.hh"
#include "starlight/core/Utils.hh"
#include "starlight/core/Log.hh"

namespace sl {

template <typename T, typename Accessor> class KeyVector {
    using Key =
      std::remove_cvref_t<decltype(std::declval<Accessor>()(std::declval<T>()))>;

public:
    KeyVector() = default;

    KeyVector(const KeyVector& oth) : m_buffer(oth.m_buffer) { rebuildView(); }
    KeyVector& operator=(const KeyVector& oth) {
        m_buffer = oth.m_buffer;
        rebuildView();
    }

    bool contains(const Key& key) const { return m_view.contains(key); }

    const T& at(const Key& key) const { return *m_view.at(key); }
    T& at(const Key& key) { return *m_view.at(key); }

    u64 size() const { return m_buffer.size(); }

    void push(const T& value) {
        m_buffer.push_back(value);
        rebuildView();
    }

    void push(T&& value) {
        m_buffer.push_back(std::move(value));
        rebuildView();
    }

    template <typename... Args>
    requires std::is_constructible_v<T, Args...>
    void emplace(Args&&... args) {
        m_buffer.emplace_back(std::forward<Args>(args)...);
        rebuildView();
    }

    template <typename Callback>
    requires Callable<Callback, void, T&>
    void forEach(Callback&& callback) {
        for (auto& element : m_buffer) callback(element);
    }

    template <typename Callback>
    requires Callable<Callback, void, const T&>
    void forEach(Callback&& callback) const {
        for (auto& element : m_buffer) callback(element);
    }

    template <typename Comparator>
    requires Callable<Comparator, bool, const T&, const T&>
    void sort(Comparator&& comparator) {
        std::ranges::sort(m_buffer, std::forward<Comparator>(comparator));
        rebuildView();
    }

    template <typename Comparator>
    requires Callable<Comparator, bool, const T&, const T&>
    void removeDuplicates(Comparator&& comparator) {
        m_buffer.erase(
          std::unique(
            m_buffer.begin(), m_buffer.end(), std::forward<Comparator>(comparator)
          ),
          m_buffer.end()
        );
        rebuildView();
    }

private:
    void rebuildView() {
        m_view.clear();
        for (auto& element : m_buffer) m_view[m_keyAccessor(element)] = &element;
    }

    Accessor m_keyAccessor;
    std::vector<T> m_buffer;
    std::unordered_map<Key, T*> m_view;
};

}  // namespace sl
