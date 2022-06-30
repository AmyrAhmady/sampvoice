/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "DynamicLocalStreamAtPoint.h"

#include <cassert>
#include <cstring>

#include <util/memory.hpp>
#include <util/logger.h>

#include "ControlPacket.h"
#include "PlayerStore.h"
#include "Header.h"

DynamicLocalStreamAtPoint::DynamicLocalStreamAtPoint(
	const float distance, const uint32_t maxPlayers,
	const Vector3& position, const uint32_t color,
	const std::string& name
)
	: LocalStream(distance)
	, DynamicStream(distance, maxPlayers)
	, PointStream(distance, position)
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);

	const auto nameString = name.c_str();
	const auto nameLength = name.size() + 1;

	PackWrap(this->packetCreateStream, SV::ControlPacketType::createLPStream, sizeof(SV::CreateLPStreamPacket) + nameLength);

	PackGetStruct(&*this->packetCreateStream, SV::CreateLPStreamPacket)->stream = reinterpret_cast<uint32_t>(static_cast<Stream*>(this));
	std::memcpy(PackGetStruct(&*this->packetCreateStream, SV::CreateLPStreamPacket)->name, nameString, nameLength);
	PackGetStruct(&*this->packetCreateStream, SV::CreateLPStreamPacket)->distance = distance;
	PackGetStruct(&*this->packetCreateStream, SV::CreateLPStreamPacket)->position = position;
	PackGetStruct(&*this->packetCreateStream, SV::CreateLPStreamPacket)->color = color;

	PlayerSortList playerList;

	for (IPlayer* player : PlayerStore::internalPlayerPool)
	{

		if (PlayerStore::IsPlayerHasPlugin(player->getID()) && distanceToPlayer <= distance)
		{
			playerList.emplace(distanceToPlayer, player->getID());
		}
	}

	for (const auto& playerInfo : playerList)
	{
		if (this->attachedListenersCount >= maxPlayers)
			break;

		this->Stream::AttachListener(playerInfo.playerId);
	}
}

void DynamicLocalStreamAtPoint::Tick()
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);

	PlayerSortList playerList;

	const Vector3& streamPosition = PackGetStruct(&*this->packetCreateStream, SV::CreateLPStreamPacket)->position;
	const float streamDistance = PackGetStruct(&*this->packetStreamUpdateDistance, SV::UpdateLStreamDistancePacket)->distance;

	for (IPlayer* player : PlayerStore::internalPlayerPool)
	{
		float distanceToPlayer = glm::distance(player->getPosition(), streamPosition);

		if (PlayerStore::IsPlayerHasPlugin(player->getID()) && distanceToPlayer <= streamDistance)
		{
			if (!this->HasListener(player->getID()))
			{
				playerList.emplace(distanceToPlayer, player->getID());
			}
		}
		else if (this->HasListener(player->getID()))
		{
			this->Stream::DetachListener(player->getID());
		}
	}

	for (const auto& playerInfo : playerList)
	{
		if (this->attachedListenersCount >= this->maxPlayers)
			break;

		this->Stream::AttachListener(playerInfo.playerId);
	}
}
