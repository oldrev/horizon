#pragma once

#include <bs_thread_pool.hpp>

using thread_pool = BS::priority_thread_pool;

inline thread_pool &GetKiCadThreadPool()
{
    static thread_pool pool;
    return pool;
}
