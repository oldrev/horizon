#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

class JSON_SETTINGS
{
public:
    virtual ~JSON_SETTINGS() = default;
};

class PARAM_BASE
{
public:
    virtual ~PARAM_BASE() = default;
};

template <typename T> class PARAM : public PARAM_BASE
{
public:
    template <typename... Args> PARAM(const char *, T *value, T defaultValue, Args &&...)
    {
        *value = defaultValue;
    }
};

template <typename T> class PARAM_ENUM : public PARAM_BASE
{
public:
    template <typename... Args> PARAM_ENUM(const char *, T *value, T defaultValue, Args &&...)
    {
        *value = defaultValue;
    }
};

template <typename T> class PARAM_LAMBDA : public PARAM_BASE
{
public:
    template <typename... Args> PARAM_LAMBDA(const char *, Args &&...)
    {
    }
};

class NESTED_SETTINGS : public JSON_SETTINGS
{
public:
    NESTED_SETTINGS(const std::string &, int, JSON_SETTINGS *, const std::string &)
    {
    }

    bool LoadFromFile() { return true; }

protected:
    std::vector<std::unique_ptr<PARAM_BASE>> m_params;
};
