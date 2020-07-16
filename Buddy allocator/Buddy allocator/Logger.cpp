#include "Logger.h"
#pragma warning(disable:4996) // asctime

void Logger::logDatetime() {
	std::time_t now = std::time(nullptr);

	char dateStr[64];
	char timeStr[64];
	std::strftime(dateStr, sizeof(dateStr), "%d.%m.%y", std::localtime(&now));
	std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&now));
	out << dateStr << ", ";
	out << timeStr << ", ";
}

void Logger::logSeverityLevel(SeverityLevel level)
{
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
	case Action::exception:
		out << "exception";
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
