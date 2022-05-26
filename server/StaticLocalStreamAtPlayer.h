/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <string>
#include <cstdint>

#include "LocalStream.h"

class StaticLocalStreamAtPlayer : public LocalStream {

	StaticLocalStreamAtPlayer() = delete;
	StaticLocalStreamAtPlayer(const StaticLocalStreamAtPlayer&) = delete;
	StaticLocalStreamAtPlayer(StaticLocalStreamAtPlayer&&) = delete;
	StaticLocalStreamAtPlayer& operator=(const StaticLocalStreamAtPlayer&) = delete;
	StaticLocalStreamAtPlayer& operator=(StaticLocalStreamAtPlayer&&) = delete;

public:

	explicit StaticLocalStreamAtPlayer(float distance, uint16_t playerId,
		uint32_t color, const std::string& name);

	~StaticLocalStreamAtPlayer() noexcept = default;

};
