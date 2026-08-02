#pragma once

#include <stddef.h>

#include "new.h"
#include "type_traits.h"

extern "C"
{
    void *kmalloc(size_t size);
    void kfree(void *ptr);
}

namespace kx
{

    template <typename T>
    class Vector
    {
    public:
        Vector() = default;
        ~Vector() { clear_and_free(); }

        Vector(const Vector &) = delete;
        Vector &operator=(const Vector &) = delete;

        Vector(Vector &&other) noexcept
            : data_(other.data_), size_(other.size_), cap_(other.cap_)
        {
            other.data_ = nullptr;
            other.size_ = 0;
            other.cap_ = 0;
        }

        Vector &operator=(Vector &&other) noexcept
        {
            if (this != &other)
            {
                clear_and_free();
                data_ = other.data_;
                size_ = other.size_;
                cap_ = other.cap_;
                other.data_ = nullptr;
                other.size_ = 0;
                other.cap_ = 0;
            }
            return *this;
        }

        size_t size() const { return size_; }
        size_t capacity() const { return cap_; }
        bool empty() const { return size_ == 0; }

        T &operator[](size_t i) { return data_[i]; }
        const T &operator[](size_t i) const { return data_[i]; }

        T &front() { return data_[0]; }
        T &back() { return data_[size_ - 1]; }

        T *begin() { return data_; }
        T *end() { return data_ + size_; }
        const T *begin() const { return data_; }
        const T *end() const { return data_ + size_; }

        void reserve(size_t new_cap)
        {
            if (new_cap <= cap_)
                return;
            T *new_data = static_cast<T *>(kmalloc(new_cap * sizeof(T)));
            for (size_t i = 0; i < size_; ++i)
            {
                new (&new_data[i]) T(move(data_[i]));
                data_[i].~T();
            }
            if (data_)
                kfree(data_);
            data_ = new_data;
            cap_ = new_cap;
        }

        void push_back(const T &value) { emplace_back(value); }
        void push_back(T &&value) { emplace_back(move(value)); }

        template <typename... Args>
        T &emplace_back(Args &&...args)
        {
            if (size_ == cap_)
                grow();
            T *slot = &data_[size_];
            new (slot) T(forward<Args>(args)...);
            ++size_;
            return *slot;
        }

        void pop_back()
        {
            if (size_ == 0)
                return;
            --size_;
            data_[size_].~T();
        }

        void clear()
        {
            for (size_t i = 0; i < size_; ++i)
                data_[i].~T();
            size_ = 0;
        }

    private:
        void grow() { reserve(cap_ == 0 ? 4 : cap_ * 2); }

        void clear_and_free()
        {
            clear();
            if (data_)
                kfree(data_);
            data_ = nullptr;
            cap_ = 0;
        }

        T *data_ = nullptr;
        size_t size_ = 0;
        size_t cap_ = 0;
    };

}
