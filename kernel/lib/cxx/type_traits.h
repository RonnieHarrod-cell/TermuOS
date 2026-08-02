#pragma once

namespace kx
{
    template <typename T>
    struct remove_reference
    {
        using type = T;
    };
    template <typename T>
    struct remove_reference<T &>
    {
        using type = T;
    };
    template <typename T>
    struct remove_reference<T &&>
    {
        using type = T;
    };

    template <typename T>
    using remove_reference_t = typename remove_reference<T>::type;

    template <typename T>
    constexpr T &&forward(remove_reference_t<T> &t) noexcept
    {
        return static_cast<T &&>(t);
    }

    template <typename T>
    constexpr T &&forward(remove_reference_t<T> &&t) noexcept
    {
        return static_cast<T &&>(t);
    }

    template <typename T>
    constexpr remove_reference_t<T> &&move(T &&t) noexcept
    {
        return static_cast<remove_reference_t<T> &&>(t);
    }
}
