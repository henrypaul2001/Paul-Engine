#pragma once
#include <chrono>
#include <iostream>
#include <thread>
#include "Profiler.h"
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