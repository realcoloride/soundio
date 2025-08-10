#ifndef INCLUDE_H 
#define INCLUDE_H

#pragma once

/* MINIAUDIO */
#define MINIAUDIO_IMPLEMENTATION 
#include "./miniaudio.h"

/* STD */
#include <stdio.h>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <memory>
#include <type_traits>
#include <iostream>
#include <thread>
#include <functional>
#include <unordered_set>
#include <algorithm>
#include <cstring>

#ifndef SI_LOG
#define SI_LOG(msg) do { \
    std::ostringstream _si_oss; \
    _si_oss << "[SI] [" << std::this_thread::get_id() << "] " << msg; \
    std::cout << _si_oss.str() << std::endl; \
} while(0)
#endif

#endif // INCLUDE_H