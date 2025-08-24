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
#include <atomic>

#ifndef SOUNDIO_LOG_ENABLED
	#define SOUNDIO_LOG_ENABLED 0
#endif

#if SOUNDIO_LOG_ENABLED
	#define SI_LOG(msg) do { std::cout << "[SI] " << msg << std::endl; } while(0)
#else
	#define SI_LOG(msg) do {} while(0)
#endif

#endif // INCLUDE_H