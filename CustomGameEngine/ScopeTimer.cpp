#include "ScopeTimer.h"
namespace Engine::Profiling {
	ScopeTimer::ScopeTimer(const char* name) : name(name)
	{
		startTimepoint = std::chrono::high_resolution_clock::now();
	}

	ScopeTimer::~ScopeTimer()
	{
		Stop();
	}

	void ScopeTimer::Stop()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		std::chrono::microseconds start = std::chrono::time_point_cast<std::chrono::microseconds>(startTimepoint).time_since_epoch();
		std::chrono::microseconds end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch();

		std::chrono::duration<double, std::micro> duration = end - start;
		double ms = duration.count() * 0.001;

		std::cout << ms << "ms: " << name << std::endl;
	}
}