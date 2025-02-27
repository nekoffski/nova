#pragma once

#include <unordered_map>

#include "Singleton.hh"
#include "Id.hh"
#include "Concepts.hh"
#include "memory/SharedPtr.hh"

namespace sl {

template <typename CFactory, typename T>
requires HasName<T>
class Factory : public Singleton<CFactory> {
public:
    SharedPtr<T> find(const std::string& key) {
        if (auto record = m_lut.find(key); record != m_lut.end())
            return record->second;
        return nullptr;
    }

    void erase(const std::string& key) {
        log::debug("Erasing '{}' from factory", key);
        m_lut.erase(key);
    }

    template <typename Callback>
    requires Callable<Callback, void, SharedPtr<T>>
    void forEach(Callback&& callback) {
        // for (auto& record : m_lut | std::views::values) callback(record);
    }

protected:
    SharedPtr<T> save(SharedPtr<T> resource) {
        auto record = m_lut.insert({ resource->name, resource });
        return record.first->second;
    }

private:
    std::unordered_map<std::string, SharedPtr<T>> m_lut;
};

}  // namespace sl
