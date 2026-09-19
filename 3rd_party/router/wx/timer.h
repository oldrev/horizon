#pragma once

#include <chrono>

class wxLongLong
{
public:
    explicit wxLongLong(long long value) : m_value(value) {}
    long long GetValue() const { return m_value; }

private:
    long long m_value;
};

inline wxLongLong wxGetLocalTimeMillis()
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return wxLongLong(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

