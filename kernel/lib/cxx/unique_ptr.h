#pragma once

#include "type_traits.h"

namespace kx
{

    template <typename T>
    struct DefaultDelete
    {
        void operator()(T *ptr) const { delete ptr; }
    };

    template <typename T>
    struct DefaultDelete<T[]>
    {
        void operator()(T *ptr) const { delete[] ptr; }
    };

    template <typename T, typename Deleter = DefaultDelete<T>>
    class UniquePtr
    {
    public:
        UniquePtr() = default;
        explicit UniquePtr(T *ptr) : ptr_(ptr) {}

        UniquePtr(const UniquePtr &) = delete;
        UniquePtr &operator=(const UniquePtr &) = delete;

        UniquePtr(UniquePtr &&other) noexcept : ptr_(other.ptr_)
        {
            other.ptr_ = nullptr;
        }

        UniquePtr &operator=(UniquePtr &&other) noexcept
        {
            if (this != &other)
            {
                reset(other.ptr_);
                other.ptr_ = nullptr;
            }
            return *this;
        }

        ~UniquePtr() { reset(); }

        T *get() const { return ptr_; }
        T *operator->() const { return ptr_; }
        T &operator*() const { return *ptr_; }
        explicit operator bool() const { return ptr_ != nullptr; }

        T *release()
        {
            T *p = ptr_;
            ptr_ = nullptr;
            return p;
        }

        void reset(T *ptr = nullptr)
        {
            if (ptr_ != ptr)
            {
                T *old = ptr_;
                ptr_ = ptr;
                if (old)
                    Deleter{}(old);
            }
        }

    private:
        T *ptr_ = nullptr;
    };

    template <typename T, typename... Args>
    UniquePtr<T> make_unique(Args &&...args)
    {
        return UniquePtr<T>(new T(forward<Args>(args)...));
    }

}
