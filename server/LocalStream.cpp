/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "LocalStream.h"

#include <cassert>

#include "NetHandler.h"
#include "PlayerStore.h"
#include "Header.h"

LocalStream::LocalStream(const float distance)
{
	PackWrap(this->packetStreamUpdateDistance, SV::ControlPacketType::updateLStreamDistance, sizeof(SV::UpdateLStreamDistancePacket));

	PackGetStruct(&*this->packetStreamUpdateDistance, SV::UpdateLStreamDistancePacket)->stream = reinterpret_cast<uint32_t>(static_cast<Stream*>(this));
	PackGetStruct(&*this->packetStreamUpdateDistance, SV::UpdateLStreamDistancePacket)->distance = distance;
}

void LocalStream::UpdateDistance(const float distance)
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);

	PackGetStruct(&*this->packetStreamUpdateDistance, SV::UpdateLStreamDistancePacket)->distance = distance;

	IPlayerPool* playerPool = SampVoiceComponent::GetPlayers();
	for (IPlayer* player : playerPool->entries())
	{
		if (this->HasListener(player->getID()) && PlayerStore::IsPlayerConnected(player->getID()))
			NetHandler::SendControlPacket(player->getID(), *&*this->packetStreamUpdateDistance);
	}
}
