/*
	This is a SampVoice project file
	Developer: CyberMor <cyber.mor.2020@gmail.ru>

	See more here https://github.com/CyberMor/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "logger.h"
#include "sdk.hpp"

bool Logger::Init(const char* const logFile, ICore* ompCore) noexcept
{
	if (logFile == nullptr || *logFile == '\0' || ompCore == nullptr)
		return false;

	const std::lock_guard<std::mutex> lockFile{ Logger::logFileMutex };
	const std::lock_guard<std::mutex> lockConsole{ Logger::logConsoleMutex };

	if (Logger::logFile != nullptr || Logger::ompCore != nullptr)
		return false;

	return (Logger::logFile = std::fopen(logFile, "wt")) != nullptr &&
		(Logger::ompCore = ompCore) != nullptr;
}

void Logger::Free() noexcept
{
	const std::lock_guard<std::mutex> lockFile{ Logger::logFileMutex };
	const std::lock_guard<std::mutex> lockConsole{ Logger::logConsoleMutex };

	if (Logger::logFile != nullptr) std::fclose(Logger::logFile);

	Logger::logFile = nullptr;
	Logger::ompCore = nullptr;
}

FILE* Logger::logFile{ nullptr };
ICore* Logger::ompCore{ nullptr };

std::mutex Logger::logFileMutex;
std::mutex Logger::logConsoleMutex;
