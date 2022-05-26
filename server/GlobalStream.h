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

#include "Stream.h"

class GlobalStream : public Stream {

	GlobalStream() = delete;
	GlobalStream(const GlobalStream&) = delete;
	GlobalStream(GlobalStream&&) = delete;
	GlobalStream& operator=(const GlobalStream&) = delete;
	GlobalStream& operator=(GlobalStream&&) = delete;

public:

	explicit GlobalStream(uint32_t color, const std::string& name);

	~GlobalStream() noexcept = default;

};
