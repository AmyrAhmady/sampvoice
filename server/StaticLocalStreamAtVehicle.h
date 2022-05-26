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

class StaticLocalStreamAtVehicle : public LocalStream {

	StaticLocalStreamAtVehicle() = delete;
	StaticLocalStreamAtVehicle(const StaticLocalStreamAtVehicle&) = delete;
	StaticLocalStreamAtVehicle(StaticLocalStreamAtVehicle&&) = delete;
	StaticLocalStreamAtVehicle& operator=(const StaticLocalStreamAtVehicle&) = delete;
	StaticLocalStreamAtVehicle& operator=(StaticLocalStreamAtVehicle&&) = delete;

public:

	explicit StaticLocalStreamAtVehicle(float distance, uint16_t vehicleId,
		uint32_t color, const std::string& name);

	~StaticLocalStreamAtVehicle() noexcept = default;

};
