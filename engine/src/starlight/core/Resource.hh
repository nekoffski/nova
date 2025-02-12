#pragma once

#include <unordered_map>

#include "Core.hh"
#include "Id.hh"
#include "Log.hh"
#include "memory/Memory.hh"

namespace sl {

template <typename T> class ResourceFactory;

template <typename T> class ResourceRef {
public:
    struct Hash {
        size_t operator()(const ResourceRef<T>& ref) const {
            return std::hash<const T*>()(ref.get());
        }
    };

    explicit ResourceRef(
      T* resource, ResourceFactory<T>* manager, const std::string& name
    );

    ResourceRef(std::nullptr_t);
    ResourceRef();

    ResourceRef(const ResourceRef& oth);
    ResourceRef(ResourceRef&& oth);

    // for compatibility with interface, lifetime managed by caller not by
    // reference counter
    ResourceRef(T* resource, const std::string& name);

    ResourceRef& operator=(const ResourceRef& oth);
    ResourceRef& operator=(ResourceRef&& oth);

    bool operator==(const ResourceRef& oth) const {
        return m_resource == oth.m_resource;
    }

    ResourceRef<T> getWeakRef();

    ~ResourceRef();

    std::string getName() const;

    operator bool() const;

    operator T*();
    operator T*() const;

    T* operator->();
    const T* operator->() const;

    T* get();
    const T* get() const;

private:
    T* m_resource;
    ResourceFactory<T>* m_manager;
    std::optional<std::string> m_name;
};

template <typename T> class ResourceFactory {
    friend class ResourceRef<T>;

    struct ResourceRecord {
        UniquePointer<T> data;
        std::string name;
        u32 referenceCounter;
    };

public:
    explicit ResourceFactory(const std::string& resourceName
    ) : m_resourceName(resourceName) {}

    ResourceRef<T> find(const std::string& name) {
        if (auto record = m_records.find(name); record != m_records.end())
            return ResourceRef<T>(record->second.data.get(), this, name);
        return nullptr;
    }

    template <typename U = T>
    requires std::is_same_v<U, T> && IsIdentificable<U>
    ResourceRef<T> find(const u64 id) {
        if (auto record = m_recordsById.find(id); record != m_recordsById.end()) {
            auto resource = record->second;
            return ResourceRef<T>(resource->data.get(), this, resource->name);
        }
        return nullptr;
    }

    const std::vector<ResourceRef<T>>& getAll() { return m_view; }

protected:
    template <typename V>
    requires std::is_same_v<V, T> && IsIdentificable<V>
    ResourceRef<V> store(UniquePointer<V> resource) {
        const auto name = fmt::format("{}_{}", m_resourceName, resource->getId());
        return store(name, std::move(resource));
    }

    ResourceRef<T> store(const std::string& name, UniquePointer<T> resource) {
        const auto [it, inserted] =
          m_records.emplace(name, ResourceRecord{ std::move(resource), name, 0u });

        if (not inserted) {
            log::warn("Record with name='{}' already exists", name);
            return nullptr;
        }

        auto& record = it->second;

        if constexpr (IsIdentificable<T>) {
            const auto id            = record.data->getId();
            const auto [_, inserted] = m_recordsById.emplace(id, &record);

            log::expect(
              inserted, "Map 'id' -> 'record' desynchronized with main buffer"
            );
        }
        ResourceRef<T> ref{ record.data.get(), this, name };
        m_view.push_back(ref);
        return ref;
    }

    void release(const std::string& name) {
        if (auto it = m_records.find(name); it != m_records.end()) {
            if (auto& record = it->second; --record.referenceCounter <= 0) {
                // 1 instance for view vector
                log::info(
                  "Reference counter of Resource {} less or equals 1, destroying",
                  name
                );
                std::erase_if(m_view, [&](auto& resource) -> bool {
                    return resource.get() == record.data.get();
                });
                if constexpr (IsIdentificable<T>)
                    m_recordsById.erase(record.data->getId());
                m_records.erase(name);
            }
        } else {
            log::warn("Could not find record to release with name: {}", name);
        }
    }

    bool acquire(const std::string& name) {
        if (auto record = m_records.find(name); record != m_records.end()) {
            record->second.referenceCounter++;
            return true;
        }
        return false;
    }

private:
    std::string m_resourceName;

    std::unordered_map<std::string, ResourceRecord> m_records;
    std::unordered_map<u64, ResourceRecord*> m_recordsById;
    std::vector<ResourceRef<T>> m_view;
};

template <typename T>
ResourceRef<T>::ResourceRef(
  T* resource, ResourceFactory<T>* manager, const std::string& name
) : m_resource(resource), m_manager(manager), m_name(name) {
    log::trace("Creating resource ref: {}", name);
    log::expect(
      resource != nullptr,
      "Attempt to create ResourceRef with resource == nullptr, {}", name
    );
    log::expect(
      manager != nullptr,
      "Attempt to create ResourceRef with manager == nullptr, {}", name
    );
    log::expect(manager->acquire(name), "Could not acquire resource: {}", name);
}

template <typename T> std::string ResourceRef<T>::getName() const {
    return m_name.value_or("<undefined>");
}

template <typename T> ResourceRef<T>::ResourceRef(std::nullptr_t) : ResourceRef() {}

template <typename T>
ResourceRef<T>::ResourceRef::ResourceRef(T* resource, const std::string& name) :
    m_resource(resource), m_manager(nullptr), m_name(name) {}

template <typename T>
ResourceRef<T>::ResourceRef() : m_resource(nullptr), m_manager(nullptr) {}

template <typename T> ResourceRef<T>::~ResourceRef() {
    if (m_manager) m_manager->release(*m_name);
}

template <typename T>
ResourceRef<T>::ResourceRef(const ResourceRef<T>& oth
) : m_resource(oth.m_resource), m_manager(oth.m_manager), m_name(oth.m_name) {
    if (m_manager) m_manager->acquire(*m_name);
}

template <typename T>
ResourceRef<T>::ResourceRef(ResourceRef&& oth) :
    m_resource(std::exchange(oth.m_resource, nullptr)),
    m_manager(std::exchange(oth.m_manager, nullptr)),
    m_name(std::exchange(oth.m_name, std::nullopt)) {}

template <typename T>
ResourceRef<T>& ResourceRef<T>::operator=(const ResourceRef<T>& oth) {
    if (m_manager) m_manager->release(*m_name);

    m_manager  = oth.m_manager;
    m_resource = oth.m_resource;
    m_name     = oth.m_name;

    if (m_manager) m_manager->acquire(*m_name);

    return *this;
}

template <typename T>
ResourceRef<T>& ResourceRef<T>::operator=(ResourceRef<T>&& oth) {
    m_manager     = oth.m_manager;
    m_resource    = oth.m_resource;
    m_name        = oth.m_name;
    oth.m_manager = nullptr;
    return *this;
}

template <typename T> T* ResourceRef<T>::operator->() { return m_resource; }

template <typename T> const T* ResourceRef<T>::operator->() const {
    return m_resource;
}

template <typename T> ResourceRef<T> ResourceRef<T>::getWeakRef() {
    return ResourceRef<T>{ m_resource, m_name.value_or("") };
}

template <typename T> T* ResourceRef<T>::get() { return m_resource; }
template <typename T> const T* ResourceRef<T>::get() const { return m_resource; }

template <typename T> ResourceRef<T>::operator T*() const { return m_resource; }
template <typename T> ResourceRef<T>::operator T*() { return m_resource; }

template <typename T> ResourceRef<T>::operator bool() const {
    return m_resource != nullptr;
}

}  // namespace sl