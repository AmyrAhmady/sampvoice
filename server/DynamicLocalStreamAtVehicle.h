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

class DynamicLocalStreamAtVehicle : public DynamicStream {

	DynamicLocalStreamAtVehicle() = delete;
	DynamicLocalStreamAtVehicle(const DynamicLocalStreamAtVehicle&) = delete;
	DynamicLocalStreamAtVehicle(DynamicLocalStreamAtVehicle&&) = delete;
	DynamicLocalStreamAtVehicle& operator=(const DynamicLocalStreamAtVehicle&) = delete;
	DynamicLocalStreamAtVehicle& operator=(DynamicLocalStreamAtVehicle&&) = delete;

public:

	explicit DynamicLocalStreamAtVehicle(float distance, uint32_t maxPlayers,
		uint16_t vehicleId, uint32_t color,
		const std::string& name);

	~DynamicLocalStreamAtVehicle() noexcept = default;

public:

	void Tick() override;

};
