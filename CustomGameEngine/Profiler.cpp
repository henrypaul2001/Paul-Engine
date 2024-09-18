#include "Profiler.h"
#include <algorithm>
namespace Engine::Profiling {
	Profiler* Profiler::instance = nullptr;
	Profiler::Profiler() : profileCount(0), activeSession(false)
	{
		std::filesystem::path directoryPath = "Profiling";

		// Create directory if it doesn't already exist
		if (!std::filesystem::exists(directoryPath)) {
			std::filesystem::create_directory(directoryPath);
			std::cout << "Profiler::Profiler::Created directory: " << "Profiling" << std::endl;
		}
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
		sessionStart = std::chrono::high_resolution_clock::now();
		
		// Get system timestamp for filename and directory
		std::time_t startTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm dateTime;
		localtime_s(&dateTime, &startTime);
		std::ostringstream dateOSS;
		dateOSS << std::put_time(&dateTime, "%d-%m-%Y");
		std::string dateString = dateOSS.str();
		std::ostringstream timeOSS;
		timeOSS << std::put_time(&dateTime, "%H-%M-%S");
		std::string timeString = timeOSS.str();

		if (filepath == "Profiling/") {
			std::filesystem::path directoryPath = filepath + dateString;

			// Create directory if it doesn't already exist
			if (!std::filesystem::exists(directoryPath)) {
				std::filesystem::create_directory(directoryPath);
				std::cout << "Profiler::BeginSession::Created directory: " << filepath + dateString << std::endl;
			}

			outputStream.open(filepath + dateString + "/profile_" + timeString + "_" + dateString + ".json");
		}
		else {
			outputStream.open(filepath);
		}
		WriteHeader();
		sessionName = name;
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
			if (profileCount > 0) { outputStream << ","; }
			profileCount++;

			std::string name = profile.profileName;
			std::replace(name.begin(), name.end(), '"', '\'');

			outputStream << "{";
			outputStream << "\"cat\":\"function\",";
			outputStream << "\"dur\":" << (profile.end - profile.start) << ',';
			outputStream << "\"name\":\"" << name << "\",";
			outputStream << "\"ph\":\"X\",";
			outputStream << "\"pid\":0,";
			outputStream << "\"tid\":" << profile.threadID << ",";
			outputStream << "\"ts\":" << profile.start;
			outputStream << "}";

			outputStream.flush();
		}
	}
}