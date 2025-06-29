#pragma once

namespace Faint {
	struct LogEntry {
		std::string time;
		std::string message;
		std::string log;
		std::string type;
		uint32_t count;
	};

	class Log {
	private:
		static const int MAX_LOG = 64;
		static std::vector<LogEntry> m_logs;

	public:
		static void Init();

		static void GetLog(const std::string& log, const std::string& logName = "FAINT", const std::string& logType = "trace");
		static std::vector<LogEntry> GetLogs();
		static void ClearLogs();
	};
}

// Core log macros
#define HZ_CORE_TRACE(log, ...) ::Faint::Log::GetLog(log, "FAINT", "trace");
#define HZ_CORE_ERROR(log, ...) ::Faint::Log::GetLog(log, "FAINT", "error");
#define HZ_CORE_WARN(log, ...)  ::Faint::Log::GetLog(log, "FAINT", "warn");
#define HZ_CORE_INFO(log, ...)  ::Faint::Log::GetLog(log, "FAINT", "info");

// Client log macros
#define HZ_TRACE(log, ...)  Faint::Log::GetLog(log, "APP");
#define HZ_ERROR(log, ...)  Faint::Log::GetLog(log, "APP", "error");
#define HZ_WARN(log, ...)   Faint::Log::GetLog(log, "APP", "warn");
#define HZ_INFO(log, ...)   Faint::Log::GetLog(log, "APP", "info");