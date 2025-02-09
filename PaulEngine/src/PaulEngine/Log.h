#pragma once

#include <memory>

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace PaulEngine {
	class PAUL_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetAppLogger() { return s_AppLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_AppLogger;
	};
}

// Core lag macros
#define PE_CORE_TRACE(...) ::PaulEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PE_CORE_INFO(...)  ::PaulEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PE_CORE_WARN(...)  ::PaulEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PE_CORE_ERROR(...) ::PaulEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PE_CORE_FATAL(...) ::PaulEngine::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// App log macros
#define PE_TRACE(...) ::PaulEngine::Log::GetAppLogger()->trace(__VA_ARGS__)
#define PE_INFO(...)  ::PaulEngine::Log::GetAppLogger()->info(__VA_ARGS__)
#define PE_WARN(...)  ::PaulEngine::Log::GetAppLogger()->warn(__VA_ARGS__)
#define PE_ERROR(...) ::PaulEngine::Log::GetAppLogger()->error(__VA_ARGS__)
#define PE_FATAL(...) ::PaulEngine::Log::GetAppLogger()->fatal(__VA_ARGS__)