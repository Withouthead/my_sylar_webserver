//
// Created by xiaomaotou31 on 2022/2/8.
//

#ifndef SYLAR_WEB_SERVER_UTILS_H
#define SYLAR_WEB_SERVER_UTILS_H

#include <syscall.h>
#include <sys/types.h>
#include <coroutine>
#include <iostream>
#include <thread>

size_t getThreadId();
size_t getFiberId();


#endif //SYLAR_WEB_SERVER_UTILS_H
