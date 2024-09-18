#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
namespace Engine::Profiling {
	struct ProfileResult {
		ProfileResult(const char* name, const long long start, const long long end, const uint32_t threadID) : profileName(name), start(start), end(end), threadID(threadID) {}

		const char* profileName;
		const long long start, end;
		const uint32_t threadID;
	};

	class Profiler
	{
	public:
		~Profiler();

		void BeginSession(const std::string& name, const std::string& filepath = "Profiling/results.json");
		void EndSession();

		void WriteProfile(const ProfileResult& profile);

		const bool IsSessionActive() const { return activeSession; }

		static Profiler* GetInstance() {
			if (instance == nullptr) {
				instance = new Profiler();
			}
			return instance;
		}
	private:
		Profiler();

		void WriteHeader();
		void WriteFooter();

		std::chrono::time_point<std::chrono::high_resolution_clock> sessionStart;

		std::ofstream outputStream;
		std::string sessionName;
		unsigned int profileCount;
		bool activeSession;

		static Profiler* instance;
	};
}