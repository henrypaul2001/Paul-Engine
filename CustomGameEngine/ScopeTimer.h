#pragma once
#include <chrono>
#include <iostream>
namespace Engine::Profiling {
	class ScopeTimer
	{
	public:
		ScopeTimer(const char* name);
		~ScopeTimer();

		void Stop();

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> startTimepoint;

		const char* name;
	};
}