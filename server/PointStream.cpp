/*
	This is a SampVoice project file
	Developer: CyberMor <cyber.mor.2020@gmail.ru>

	See more here https://github.com/CyberMor/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "PointStream.h"

#include <cassert>

#include "sdk.hpp"

#include "Network.h"
#include "PlayerStore.h"
#include "Header.h"

PointStream::PointStream(const float distance, const Vector3& position) : LocalStream(distance)
{
	PackWrap(this->packetStreamUpdatePosition, SV::ControlPacketType::updateLPStreamPosition, sizeof(SV::UpdateLPStreamPositionPacket));

	PackGetStruct(&*this->packetStreamUpdatePosition, SV::UpdateLPStreamPositionPacket)->stream = reinterpret_cast<uint32_t>(static_cast<Stream*>(this));
	PackGetStruct(&*this->packetStreamUpdatePosition, SV::UpdateLPStreamPositionPacket)->position = position;
}

void PointStream::UpdatePosition(const Vector3& position)
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);

	PackGetStruct(&*this->packetStreamUpdatePosition, SV::UpdateLPStreamPositionPacket)->position = position;

	IPlayerPool* playerPool = SampVoiceComponent::GetPlayers();
	for (IPlayer* player : playerPool->entries())
	{
		if (this->HasListener(player->getID()) && PlayerStore::IsPlayerConnected(player->getID()))
			Network::SendControlPacket(player->getID(), *&*this->packetStreamUpdatePosition);
	}
}
