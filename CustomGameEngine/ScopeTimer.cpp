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

		long long start = std::chrono::time_point_cast<std::chrono::microseconds>(startTimepoint).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

		uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
		Profiling::Profiler::GetInstance()->WriteProfile(ProfileResult(name, start, end, threadID));
	}
}