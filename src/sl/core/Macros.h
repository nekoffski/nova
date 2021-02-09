#pragma once

#define SL_STATIC_CLASS(T)           \
public:                              \
    T() = delete;                    \
    T(const T&) = delete;            \
    T(const T&&) = delete;           \
    T& operator=(const T&) = delete; \
    T& operator=(T&&) = delete;