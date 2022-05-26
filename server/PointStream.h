/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include "sdk.hpp"

#include "ControlPacket.h"
#include "LocalStream.h"

class PointStream : public virtual LocalStream {

	PointStream() = delete;
	PointStream(const PointStream&) = delete;
	PointStream(PointStream&&) = delete;
	PointStream& operator=(const PointStream&) = delete;
	PointStream& operator=(PointStream&&) = delete;

protected:

	explicit PointStream(float distance, const Vector3& position);

public:

	virtual ~PointStream() noexcept = default;

public:

	void UpdatePosition(const Vector3& position);

protected:

	ControlPacketContainerPtr packetStreamUpdatePosition{ nullptr };

};
