#pragma once
#include <chrono>
#include <iostream>
#include <thread>
#include "Profiler.h"

#define SCOPE_TIMER(name) Engine::Profiling::ScopeTimer scopeTimer = Engine::Profiling::ScopeTimer(name)

namespace Engine::Profiling {
	class ScopeTimer
	{
	public:
		ScopeTimer(const char* name);
		~ScopeTimer();

	private:
		void Stop();

		std::chrono::time_point<std::chrono::high_resolution_clock> startTimepoint;

		const char* name;
	};
}