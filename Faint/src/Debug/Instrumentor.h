#include <iostream>
#include <chrono>
#include <unordered_map>

#include "Debug/Log.h"

namespace Faint {
	struct ProfileResult {
		std::string name;
		long long start, end;
		uint32_t id;
	};
	struct InstrumentationSession {
		std::string name;
	};
	class Instrumentor {
	private:
		InstrumentationSession* _currentSession;
		std::ofstream _outputStream;
		int _profileCount;
	public:
		Instrumentor() : _currentSession(nullptr), _profileCount(0) {
		}

		void BeginSession(const std::string& name, const std::string& filepath = "results.json") {
			_outputStream.open(filepath);
			WriteHeader();
			_currentSession = new InstrumentationSession{ name };
		}

		void EndSession() {
			WriteFooter();
			_outputStream.close();
			delete _currentSession;
			_currentSession = nullptr;
			_profileCount = 0;
		}

		void WriteProfile(const ProfileResult& result) {
			if (_profileCount++ > 0)
				_outputStream << ",";

			std::string name = result.name;
			std::replace(name.begin(), name.end(), '"', '\'');

			_outputStream << "{";
			_outputStream << "\"cat\":\"function\",";
			_outputStream << "\"dur\":" << (result.end - result.start) << ",";
			_outputStream << "\"name\":\"" << name << "\",";
			_outputStream << "\"ph\":\"X\",";
			_outputStream << "\"pid\":0,";
			_outputStream << "\"tid\":" << result.id << ",";
			_outputStream << "\"ts\":" << result.start;
			_outputStream << "}";

			_outputStream.flush();
		}

		void WriteHeader() {
			_outputStream << "{\"otherData\": {},\"traceEvents\":[";
			_outputStream.flush();
		}
		void WriteFooter() {
			_outputStream << "]}";
			_outputStream.flush();
		}

		static Instrumentor& Get() {
			static Instrumentor instance;
			return instance;
		}
	};

	class Timer {
	public:
		Timer(const char* name) {
			_startTime = std::chrono::high_resolution_clock::now();
			_name = name;
		}
		~Timer() {
			if (!_stopped) {

				auto endTime = std::chrono::high_resolution_clock::now();

				long long start = std::chrono::time_point_cast<std::chrono::milliseconds>(_startTime).time_since_epoch().count();
				long long end = std::chrono::time_point_cast<std::chrono::milliseconds>(endTime).time_since_epoch().count();
				
				uint32_t id = (uint32_t)std::hash<std::thread::id>{}(std::this_thread::get_id());
				Instrumentor::Get().WriteProfile({ _name, start, end, id });
				
				_stopped = true;
				//std::cout << _name << ": " << (end - start) << "ms\n";
			}
		}
	private:
		std::string _name;
		std::chrono::time_point<std::chrono::steady_clock> _startTime;
		bool _stopped = false;
	};
}

#define HZ_PROFILE 1
#if HZ_PROFILE
#define HZ_PROFILE_BEGIN_SESSION(name, filepath) ::Faint::Instrumentor::Get().BeginSession(name, filepath)
#define HZ_PROFILE_END_SESSION() ::Faint::Instrumentor::Get().EndSession()
#define HZ_PROFILE_SCOPE(name) ::Faint::Timer timer##__LINE__(name);
#define HZ_PROFILE_FUNCTION() HZ_PROFILE_SCOPE(__FUNCSIG__)
#else
#define HZ_PROFILE_BEGIN_SESSION(name, filepath)
#define HZ_PROFILE_END_SESSION()
#define HZ_PROFILE_SCOPE(name)
#define HZ_PROFILE_FUNCTION()
#endif