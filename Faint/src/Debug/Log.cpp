#include "hzpch.h"
#include "Log.h"

//#include "spdlog/sinks/stdout_color_sinks.h"

namespace Faint {

	std::vector<LogEntry> Log::m_logs;

	void Log::Init() {
		 
		
	}

	void Log::GetLog(const std::string& log, const std::string& logName, const std::string& logType) {

		if (!m_logs.empty() && m_logs.back().message == log) {
			m_logs.back().count += 1;
			return;
		}

		char buff[100];
		time_t now = time(0);
		struct tm timeinfo;

#ifdef FT_PLATFORM_WINDOWS
		localtime_s(&timeinfo, &now);
		strftime(buff, 100, "%H:%M:%S", &timeinfo);
#endif

		LogEntry newLog = {
			buff,
			log,
			logName,
			0
		};

		std::string msg = "";

		if (logType == "trace") {
			msg += "\033[37m";
		}
		else if (logType == "error") {
			msg += "\033[91m";
		}
		else if (logType == "warn") {
			msg += "\033[93m";
		}
		else if (logType == "info") {
			msg += "\033[92m";
		}

		msg += "[" + std::string(buff) + "]" + " " + logName + ": " + log;
		std::cout << msg << "\033[37m\n";

		if (m_logs.size() >= MAX_LOG)
			m_logs.erase(m_logs.begin());

		m_logs.push_back(newLog);
	}
}
