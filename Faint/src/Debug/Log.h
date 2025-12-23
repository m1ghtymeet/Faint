#pragma once

#include <string>
#include <vector>
#include <format>

namespace Moon {
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
		static void GetLog(const std::string& log, const std::string& logName = "Moon", const std::string& logType = "trace");
		template<typename... Args>
		static void GetLogFmt(const std::string& log, const std::string& logName, const std::string& logType, Args&&... args) {
			GetLog(std::vformat(log, std::make_format_args(args...)), logName, logType);
		}
		static std::vector<LogEntry> GetLogs();
		static void ClearLogs();
	};
}

// Core log macros
#define HZ_CORE_TRACE(log, ...) ::Moon::Log::GetLogFmt(log, "Moon", "trace", __VA_ARGS__);
#define HZ_CORE_ERROR(log, ...) ::Moon::Log::GetLogFmt(log, "Moon", "error", __VA_ARGS__);
#define HZ_CORE_WARN(log, ...)  ::Moon::Log::GetLogFmt(log, "Moon", "warn", __VA_ARGS__);
#define HZ_CORE_INFO(log, ...)  ::Moon::Log::GetLogFmt(log, "Moon", "info", __VA_ARGS__);

// Client log macros
#define HZ_TRACE(log, ...)  Moon::Log::GetLogFmt(log, "APP", "trace", __VA_ARGS__);
#define HZ_ERROR(log, ...)  Moon::Log::GetLogFmt(log, "APP", "error", __VA_ARGS__);
#define HZ_WARN(log, ...)   Moon::Log::GetLogFmt(log, "APP", "warn", __VA_ARGS__);
#define HZ_INFO(log, ...)   Moon::Log::GetLogFmt(log, "APP", "info", __VA_ARGS__);