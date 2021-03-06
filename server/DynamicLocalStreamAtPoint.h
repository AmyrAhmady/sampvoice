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

#include "sdk.hpp"

#include "PointStream.h"
#include "DynamicStream.h"

class DynamicLocalStreamAtPoint : public PointStream, public DynamicStream {

	DynamicLocalStreamAtPoint() = delete;
	DynamicLocalStreamAtPoint(const DynamicLocalStreamAtPoint&) = delete;
	DynamicLocalStreamAtPoint(DynamicLocalStreamAtPoint&&) = delete;
	DynamicLocalStreamAtPoint& operator=(const DynamicLocalStreamAtPoint&) = delete;
	DynamicLocalStreamAtPoint& operator=(DynamicLocalStreamAtPoint&&) = delete;

public:

	explicit DynamicLocalStreamAtPoint(float distance, uint32_t maxPlayers,
		const Vector3& position, uint32_t color,
		const std::string& name);

	~DynamicLocalStreamAtPoint() noexcept = default;

public:

	void Tick() override;

};
