﻿/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <ctime>
#include <iostream>
#include <mutex>

#include "sdk.hpp"

class Logger {

	Logger() = delete;
	~Logger() = delete;
	Logger(const Logger&) = delete;
	Logger(Logger&&) = delete;
	Logger& operator=(const Logger&) = delete;
	Logger& operator=(Logger&&) = delete;

public:

	static bool Init(const char* logFile, ICore* ompCore) noexcept;
	static void Free() noexcept;

	template<class... ARGS>
	static bool LogToFile(const char* const message, const ARGS... args) noexcept
	{
		const std::lock_guard<std::mutex> lock{ Logger::logFileMutex };

		if (Logger::logFile == nullptr)
			return false;

		const auto cTime = std::time(nullptr);
		const auto timeOfDay = std::localtime(&cTime);

		if (timeOfDay == nullptr)
			return false;

		std::fprintf(Logger::logFile, "[%.2d:%.2d:%.2d] : ",
			timeOfDay->tm_hour, timeOfDay->tm_min, timeOfDay->tm_sec);
		std::fprintf(Logger::logFile, message, args...);
		std::fputc('\n', Logger::logFile);
		std::fflush(Logger::logFile);

		return true;
	}

	template<class... ARGS>
	static bool LogToConsole(const char* const message, const ARGS... args) noexcept
	{
		const std::lock_guard<std::mutex> lock{ Logger::logConsoleMutex };

		if (ompCore == nullptr)
			return false;

		ompCore->printLn(message, args...);

		return true;
	}

	template<class... ARGS>
	static inline void Log(const char* const message, const ARGS... args) noexcept
	{
		Logger::LogToFile(message, args...);
		Logger::LogToConsole(message, args...);
	}

private:

	static FILE* logFile;
	static ICore* ompCore;

	static std::mutex logFileMutex;
	static std::mutex logConsoleMutex;

};
