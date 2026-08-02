#pragma once

#include <stddef.h>

namespace kx
{

    class StringView
    {
    public:
        static constexpr size_t npos = static_cast<size_t>(-1);

        constexpr StringView() = default;
        constexpr StringView(const char *str, size_t len) : data_(str), len_(len) {}
        StringView(const char *cstr) : data_(cstr), len_(cstr ? cstr_len(cstr) : 0) {}

        const char *data() const { return data_; }
        size_t size() const { return len_; }
        bool empty() const { return len_ == 0; }

        char operator[](size_t i) const { return data_[i]; }

        const char *begin() const { return data_; }
        const char *end() const { return data_ + len_; }

        bool operator==(StringView other) const
        {
            if (len_ != other.len_)
                return false;
            for (size_t i = 0; i < len_; ++i)
            {
                if (data_[i] != other.data_[i])
                    return false;
            }
            return true;
        }
        bool operator!=(StringView other) const { return !(*this == other); }

        StringView substr(size_t pos, size_t count = npos) const
        {
            if (pos > len_)
                pos = len_;
            size_t avail = len_ - pos;
            if (count > avail)
                count = avail;
            return StringView(data_ + pos, count);
        }

        size_t find(char c) const
        {
            for (size_t i = 0; i < len_; ++i)
            {
                if (data_[i] == c)
                    return i;
            }
            return npos;
        }

    private:
        static size_t cstr_len(const char *s)
        {
            size_t n = 0;
            while (s[n])
                ++n;
            return n;
        }

        const char *data_ = nullptr;
        size_t len_ = 0;
    };

}
