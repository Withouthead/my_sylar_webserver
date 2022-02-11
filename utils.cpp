//
// Created by xiaomaotou31 on 2022/2/10.
//
#include "utils.h"
size_t getThreadId()
{
    return std::hash<std::thread::id>{}(std::this_thread::get_id());
}
size_t getFiberId()
{
    return 0; // TODO:get coroutine id
}