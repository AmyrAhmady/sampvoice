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

class StaticLocalStreamAtPoint : public PointStream {

	StaticLocalStreamAtPoint() = delete;
	StaticLocalStreamAtPoint(const StaticLocalStreamAtPoint&) = delete;
	StaticLocalStreamAtPoint(StaticLocalStreamAtPoint&&) = delete;
	StaticLocalStreamAtPoint& operator=(const StaticLocalStreamAtPoint&) = delete;
	StaticLocalStreamAtPoint& operator=(StaticLocalStreamAtPoint&&) = delete;

public:

	explicit StaticLocalStreamAtPoint(float distance, const Vector3& position,
		uint32_t color, const std::string& name);

	~StaticLocalStreamAtPoint() noexcept = default;

};
