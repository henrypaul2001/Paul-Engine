#include "Profiler.h"
#include <algorithm>
namespace Engine::Profiling {
	Profiler* Profiler::instance = nullptr;
	Profiler::Profiler() : profileCount(0), activeSession(false)
	{

	}

	Profiler::~Profiler()
	{

	}

	void Profiler::WriteHeader()
	{
		outputStream << "{\"otherData\": {},\"traceEvents\":[";
		outputStream.flush();
	}

	void Profiler::WriteFooter()
	{
		outputStream << "]}";
		outputStream.flush();
	}

	void Profiler::BeginSession(const std::string& name, const std::string& filepath)
	{
		std::cout << "PROFILER::BeginSession()" << std::endl;
		activeSession = true;
		outputStream.open(filepath);
		WriteHeader();
		sessionName = name;
		sessionStart = std::chrono::high_resolution_clock::now();
	}

	void Profiler::EndSession()
	{
		auto sessionEnd = std::chrono::high_resolution_clock::now();

		long long start = std::chrono::time_point_cast<std::chrono::milliseconds>(sessionStart).time_since_epoch().count();
		long long end = std::chrono::time_point_cast<std::chrono::milliseconds>(sessionEnd).time_since_epoch().count();

		auto duration = end - start;

		float seconds = duration * 0.001f;

		WriteFooter();
		outputStream.close();

		std::cout << "PROFILER::EndSession()" << std::endl;
		std::cout << "    - " << "Session duration: " << seconds << "s" << std::endl;
		std::cout << "    - " << profileCount << " profiles captured" << std::endl;

		profileCount = 0;
		activeSession = false;
	}

	void Profiler::WriteProfile(const ProfileResult& profile)
	{
		if (activeSession) {
			profileCount++;
			if (profileCount > 0) { outputStream << ","; }

			std::string name = profile.profileName;
			std::replace(name.begin(), name.end(), '"', '\'');

			outputStream << "{";
			outputStream << "\"cat\":\"function\",";
			outputStream << "\dur\":" << (profile.end - profile.start) << ',';
			outputStream << "\name\":\"" << name << "\",";
			outputStream << "\"ph\":\"X\",";
			outputStream << "\"pid\":0,";
			outputStream << "\"tid\":" << profile.threadID << ",";
			outputStream << "\"ts\":" << profile.start;
			outputStream << "}";

			outputStream.flush();
		}
	}
}