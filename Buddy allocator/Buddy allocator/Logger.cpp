#include "Logger.hpp"
#include <chrono>
#include <iomanip>

void Logger::logDatetime() {
	std::time_t theTime = std::time(nullptr);
	auto now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();

	/* UTC: -3:00 = 24 - 3 = 21 */
	typedef std::chrono::duration< int, std::ratio_multiply< std::chrono::hours::period, std::ratio< 21 > >::type > Days;
	Days days = std::chrono::duration_cast<Days>(duration);
	duration -= days;
	auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
	duration -= hours;
	auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
	duration -= minutes;
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
	duration -= seconds;
	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

	struct tm aTime;
	localtime_s(&aTime, &theTime);
	char dateStr[64];
	std::strftime(dateStr, sizeof(dateStr), "%d.%m.%y", &aTime);

	out << dateStr << ", "
		<< aTime.tm_hour << ":"
		<< minutes.count() << ":"
		<< seconds.count() << "."
		<< milliseconds.count() << ", ";
}

void Logger::logSeverityLevel(SeverityLevel level) {
	switch (level) {
	case SeverityLevel::info:
		out << "info";
		break;
	case SeverityLevel::warning:
		out << "warning";
		break;
	case SeverityLevel::error:
		out << "error";
		break;
	default:
		out << "unknown";
	}
	out << ", ";
}

void Logger::logAction(Action action) {
	switch (action) {
	case Action::none:
		out << "none";
		break;
	case Action::allocation:
		out << "allocation";
		break;
	case Action::deallocation:
		out << "deallocation";
		break;
	case Action::leak:
		out << "leak";
		break;
	case Action::exception:
		out << "exception";
		break;
	case Action::initialisation:
		out << "initialisation";
		break;
	default:
		out << "unknown";
	}
	out << ", ";
}

Logger::Logger(std::ostream& outputStream):out(outputStream) {
	out << "Date, Time, Severity Level, Action, Location, Message\n";
}

void Logger::_log(SeverityLevel level, Action action, const StaticString<64>& message, const char* fileName, int line) {
	logDatetime();
	logSeverityLevel(level);
	logAction(action);
	// TODO: escaping
	out << fileName << ':' << line << ", " << message << '\n';
}
