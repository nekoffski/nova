#pragma once

#include <memory>

#include <kc/mem/LocalPtr.hpp>
#include <kc/mem/OwningPtr.hpp>

namespace sl {

using kc::mem::LocalPtr;

template <typename T> using RefPtr = std::shared_ptr<T>;

template <typename T, typename... Args>
constexpr RefPtr<T> createRefPtr(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T> using UniqPtr = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr UniqPtr<T> createUniqPtr(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// todo: unify with UniqPtr, for now keep both to not break anything
template <typename T> using OwningPtr = kc::mem::OwningPtr<T>;

template <typename T, typename... Args>
constexpr OwningPtr<T> createOwningPtr(Args&&... args) {
    return kc::mem::makeOwningPtr<T>(std::forward<Args>(args)...);
}

// template <typename T> using OwningPtr = UniqPtr<T>;

// template <typename T, typename... Args>
// constexpr OwningPtr<T> createOwningPtr(Args&&... args) {
//     return createUniqPtr<T>(std::forward<Args>(args)...);
// }

}  // namespace sl