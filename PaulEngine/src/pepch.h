#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <filesystem>

#include <string>
#include <ostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <queue>
#include <array>
#include <unordered_map>
#include <unordered_set>

#include "PaulEngine/Core/Core.h"
#include "PaulEngine/Core/Buffer.h"

#include "PaulEngine/Core/Log.h"
#include "PaulEngine/Debug/Instrumentor.h"

#ifdef PE_PLATFORM_WINDOWS
	#include <Windows.h>
#endif