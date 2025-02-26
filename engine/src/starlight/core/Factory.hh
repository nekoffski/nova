#pragma once

#include <unordered_map>

#include "Singleton.hh"
#include "memory/SharedPtr.hh"

namespace sl {

template <typename CFactory, typename T> class Factory : public Singleton<CFactory> {
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

protected:
    SharedPtr<T> save(const std::string& key, SharedPtr<T> resource) {
        auto record = m_lut.insert({ key, resource });
        return record.first->second;
    }

private:
    std::unordered_map<std::string, SharedPtr<T>> m_lut;
};

}  // namespace sl
