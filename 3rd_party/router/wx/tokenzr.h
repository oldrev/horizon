#pragma once

#include "../wx_compat.h"
#include <sstream>

class wxStringTokenizer
{
public:
    explicit wxStringTokenizer(const wxString &value) : m_stream(value)
    {
    }

    wxString GetNextToken()
    {
        std::string token;
        m_stream >> token;
        return token;
    }

private:
    std::istringstream m_stream;
};

