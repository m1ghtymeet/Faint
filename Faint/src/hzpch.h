#pragma once

#ifdef FT_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
#endif

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <future>
#include <thread>
#include <numeric>

#include "Debug/Log.h"
#include "Debug/Instrumentor.h"

#ifdef FT_PLATFORM_WINDOWS
	#include <Windows.h>
#endif

