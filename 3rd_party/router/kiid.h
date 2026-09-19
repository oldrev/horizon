#pragma once

#include "wx_compat.h"

class KIID
{
public:
    KIID() = default;
    explicit KIID(const wxString &value) : m_value(value)
    {
    }

    const std::string &AsString() const { return m_value; }
    bool operator<(const KIID &other) const { return m_value < other.m_value; }

private:
    std::string m_value;
};

