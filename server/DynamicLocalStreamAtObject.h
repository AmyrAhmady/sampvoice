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

class DynamicLocalStreamAtObject : public DynamicStream {

	DynamicLocalStreamAtObject() = delete;
	DynamicLocalStreamAtObject(const DynamicLocalStreamAtObject&) = delete;
	DynamicLocalStreamAtObject(DynamicLocalStreamAtObject&&) = delete;
	DynamicLocalStreamAtObject& operator=(const DynamicLocalStreamAtObject&) = delete;
	DynamicLocalStreamAtObject& operator=(DynamicLocalStreamAtObject&&) = delete;

public:

	explicit DynamicLocalStreamAtObject(float distance, uint32_t maxPlayers,
		uint16_t objectId, uint32_t color,
		const std::string& name);

	~DynamicLocalStreamAtObject() noexcept = default;

public:

	void Tick() override;

};
