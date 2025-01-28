#pragma once

#include <limits>
#include <new>

#include "starlight/core/Log.hh"
#include "Allocator.hh"

namespace sl {

template <typename T> class Mallocator : public Allocator {
public:
    using ValueType = T;

    explicit Mallocator() = default;

    [[nodiscard]] void* allocate(u64 n) override {
        if (n > std::numeric_limits<u64>::max() / sizeof(T)) [[unlikely]]
            throw std::bad_alloc();

        if (auto p = static_cast<T*>(std::malloc(n * sizeof(T))); p) [[likely]] {
            report(p, n);
            return p;
        }

        throw std::bad_alloc();
    }

    void deallocate(void* p, u64 n) noexcept override {
        report(p, n, false);
        std::free(static_cast<T*>(p));
    }

private:
    void report(void* p, u64 n, bool alloc = true) const {
        log::trace(
          "{} '{}' bytes at '{}'",
          alloc ? "Allocating:" : "Deallocating:", sizeof(T) * n, p
        );
    }
};

}  // namespace sl
