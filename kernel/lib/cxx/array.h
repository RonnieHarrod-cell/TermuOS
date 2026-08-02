#pragma once

#include <stddef.h>

namespace kx
{

    template <typename T, size_t N>
    struct Array
    {
        T data_[N];

        constexpr size_t size() const { return N; }
        constexpr bool empty() const { return N == 0; }

        T *data() { return data_; }
        const T *data() const { return data_; }

        T &operator[](size_t i) { return data_[i]; }
        const T &operator[](size_t i) const { return data_[i]; }

        T &front() { return data_[0]; }
        T &back() { return data_[N - 1]; }

        T *begin() { return data_; }
        T *end() { return data_ + N; }
        const T *begin() const { return data_; }
        const T *end() const { return data_ + N; }
    };

}
