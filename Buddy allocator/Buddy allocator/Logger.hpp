#pragma once
#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <optional>
#include "StaticString.hpp"

class Logger
{
private:
	std::ostream& out;
public:
	enum class SeverityLevel { info, warning, error };
	enum class Action { none, allocation, deallocation, leak, exception, initialisation };
private:
	void logDatetime();
	void logSeverityLevel(SeverityLevel level);
	void logAction(Action action);
public:
	Logger(std::ostream& out);
	void _log(SeverityLevel level, Action action = Action::none, const StaticString<64>& message = "", const char * fileName = "", int line = 0);

#ifndef log
#define log(level, action, message) _log(level, action, message, __FILE__, __LINE__)
#endif // log(level, action, message)
};
