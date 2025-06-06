/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "DynamicLocalStreamAtVehicle.h"

#include <cassert>
#include <cstring>

#include <util/memory.hpp>
#include <util/logger.h>

#include "ControlPacket.h"
#include "PlayerStore.h"
#include "Header.h"

DynamicLocalStreamAtVehicle::DynamicLocalStreamAtVehicle(
	const float distance, const uint32_t maxPlayers,
	const uint16_t vehicleId, const uint32_t color,
	const std::string& name
)
	: LocalStream(distance)
	, DynamicStream(distance, maxPlayers)
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);
	assert(SampVoiceComponent::GetVehicles() != nullptr);

	const auto nameString = name.c_str();
	const auto nameLength = name.size() + 1;

	PackWrap(this->packetCreateStream, SV::ControlPacketType::createLStreamAtVehicle, sizeof(SV::CreateLStreamAtPacket) + nameLength);

	PackGetStruct(&*this->packetCreateStream, SV::CreateLStreamAtPacket)->stream = reinterpret_cast<uint32_t>(static_cast<Stream*>(this));
	std::memcpy(PackGetStruct(&*this->packetCreateStream, SV::CreateLStreamAtPacket)->name, nameString, nameLength);
	PackGetStruct(&*this->packetCreateStream, SV::CreateLStreamAtPacket)->distance = distance;
	PackGetStruct(&*this->packetCreateStream, SV::CreateLStreamAtPacket)->target = vehicleId;
	PackGetStruct(&*this->packetCreateStream, SV::CreateLStreamAtPacket)->color = color;

	IVehicle* vehicle = SampVoiceComponent::GetVehicles()->get(vehicleId);
	if (vehicle != nullptr)
	{
		PlayerSortList playerList;

		const Vector3& streamPosition = vehicle->getPosition();

		for (IPlayer* player : PlayerStore::internalPlayerPool)
		{
			if (PlayerStore::IsPlayerHasPlugin(player->getID()) && (vehicle->isStreamedInForPlayer(*player) || player->getSpectateData().spectateID != INVALID_VEHICLE_ID))
			{
				float distanceToPlayer = glm::distance(player->getPosition(), streamPosition);
				if (distanceToPlayer <= distance) 
				{
					playerList.emplace(distanceToPlayer, player->getID());
				}
			}
		}

		for (const auto& playerInfo : playerList)
		{
			if (this->attachedListenersCount >= maxPlayers)
				break;

			this->Stream::AttachListener(playerInfo.playerId);
		}
	}
}

void DynamicLocalStreamAtVehicle::Tick()
{
	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);
	assert(SampVoiceComponent::GetVehicles() != nullptr);

	const auto vehicleId = PackGetStruct(&*this->packetCreateStream, SV::CreateLStreamAtPacket)->target;

	IVehicle* vehicle = SampVoiceComponent::GetVehicles()->get(vehicleId);
	if (vehicle != nullptr)
	{
		PlayerSortList playerList;

		const Vector3& streamPosition = vehicle->getPosition();
		const float streamDistance = PackGetStruct(&*this->packetStreamUpdateDistance, SV::UpdateLStreamDistancePacket)->distance;

		for (IPlayer* player : PlayerStore::internalPlayerPool)
		{
			if (PlayerStore::IsPlayerHasPlugin(player->getID()) && (vehicle->isStreamedInForPlayer(*player) || player->getSpectateData().spectateID != INVALID_VEHICLE_ID))
			{
				float distanceToPlayer = glm::distance(player->getPosition(), streamPosition);
				if (distanceToPlayer <= streamDistance)
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
}
