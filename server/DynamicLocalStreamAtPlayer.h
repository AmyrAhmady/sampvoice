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

#include "DynamicStream.h"

class DynamicLocalStreamAtPlayer : public DynamicStream {

	DynamicLocalStreamAtPlayer() = delete;
	DynamicLocalStreamAtPlayer(const DynamicLocalStreamAtPlayer&) = delete;
	DynamicLocalStreamAtPlayer(DynamicLocalStreamAtPlayer&&) = delete;
	DynamicLocalStreamAtPlayer& operator=(const DynamicLocalStreamAtPlayer&) = delete;
	DynamicLocalStreamAtPlayer& operator=(DynamicLocalStreamAtPlayer&&) = delete;

public:

	explicit DynamicLocalStreamAtPlayer(float distance, uint32_t maxPlayers,
		uint16_t playerId, uint32_t color,
		const std::string& name);

	~DynamicLocalStreamAtPlayer() noexcept = default;

public:

	void Tick() override;

};
