/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "DynamicStream.h"

DynamicStream::DynamicStream(const float distance, const uint32_t maxPlayers)
	: LocalStream(distance), maxPlayers(maxPlayers) {}

bool DynamicStream::AttachListener(uint16_t) noexcept { return false; }
bool DynamicStream::DetachListener(uint16_t) noexcept { return false; }

std::vector<uint16_t> DynamicStream::DetachAllListeners() noexcept
{
	return std::vector<uint16_t>();
}
