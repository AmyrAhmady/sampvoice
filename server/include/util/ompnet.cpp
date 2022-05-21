/*
	This is a SampVoice project file
	Developer: CyberMor <cyber.mor.2020@gmail.ru>

	See more here https://github.com/CyberMor/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "ompnet.h"
#include "../Header.h"
#include "packet.hpp"
#include "logger.h"

OmpNet* OmpNet::instance = nullptr;

bool OmpNet::Init(ICore* core) noexcept
{
	if (OmpNet::initStatus) return false;

	Logger::Log("[dbg:raknet:init] : module initializing...");
	OmpNet::ompCore = core;
	OmpNet::loadStatus = false;

	Logger::Log("[dbg:raknet:init] : module initialized");

	OmpNet::initStatus = true;

	return true;
}

void OmpNet::Free() noexcept
{
	if (!OmpNet::initStatus) return;

	Logger::Log("[dbg:raknet:free] : module releasing...");

	OmpNet::loadStatus = false;

	for (uint16_t playerId{ 0 }; playerId < PLAYER_POOL_SIZE; ++playerId)
	{
		if (OmpNet::playerStatus[playerId])
		{
			for (const auto& disconnectCallback : OmpNet::disconnectCallbacks)
			{
				if (disconnectCallback != nullptr) disconnectCallback(playerId);
			}
		}

		OmpNet::playerStatus[playerId] = false;
	}

	OmpNet::connectCallbacks.clear();
	OmpNet::packetCallbacks.clear();
	OmpNet::disconnectCallbacks.clear();

	OmpNet::hookDisconnect.reset();
	OmpNet::hookGetRakServerInterface.reset();

	Logger::Log("[dbg:raknet:free] : module released");

	OmpNet::initStatus = false;
}

bool OmpNet::IsLoaded() noexcept
{
	return OmpNet::loadStatus;
}

void OmpNet::Process() noexcept
{
	// Rpc's sending...
	{
		const std::unique_lock<std::shared_mutex> lock{ OmpNet::rpcQueueMutex };

		SendRpcInfo sendRpcInfo;

		while (OmpNet::rpcQueue.try_pop(sendRpcInfo))
		{
			const bool broadCast = sendRpcInfo.playerId == 0xffff;
			if (broadCast)
			{
				for (INetwork* network : ompCore->getNetworks())
				{
					network->broadcastRPC(
						sendRpcInfo.rpcId, Span<uint8_t>(sendRpcInfo.bitStream.get()->GetData(),
							sendRpcInfo.bitStream.get()->GetNumberOfBitsUsed()), OrderingChannel::OrderingChannel_Reliable, nullptr
					);
				}
			}
			else
			{
				IPlayer* player = SampVoiceComponent::GetPlayers()->get(sendRpcInfo.playerId);
				if (player)
				{
					player->sendRPC(sendRpcInfo.rpcId, Span<uint8_t>(sendRpcInfo.bitStream.get()->GetData(), sendRpcInfo.bitStream.get()->GetNumberOfBitsUsed()), OrderingChannel::OrderingChannel_Reliable);
				}
			}
		}
	}

	// Packets sending...
	{
		const std::unique_lock<std::shared_mutex> lock{ OmpNet::packetQueueMutex };

		SendPacketInfo sendPacketInfo;

		while (OmpNet::packetQueue.try_pop(sendPacketInfo))
		{
			const bool broadCast = sendPacketInfo.playerId == 0xffff;
			if (broadCast)
			{
				for (INetwork* network : ompCore->getNetworks())
				{
					network->broadcastPacket(Span<uint8_t>(sendPacketInfo.bitStream.get()->GetData(), sendPacketInfo.bitStream.get()->GetNumberOfBitsUsed()), OrderingChannel::OrderingChannel_Reliable, nullptr);
				}
			}
			else
			{
				IPlayer* player = SampVoiceComponent::GetPlayers()->get(sendPacketInfo.playerId);
				if (player)
				{
					player->sendPacket(Span<uint8_t>(sendPacketInfo.bitStream.get()->GetData(), sendPacketInfo.bitStream.get()->GetNumberOfBitsUsed()), OrderingChannel::OrderingChannel_Reliable);
				}
			}
		}
	}

	// Kicking players...
	{
		const std::unique_lock<std::shared_mutex> lock{ OmpNet::kickQueueMutex };

		uint16_t kickPlayerId;

		while (OmpNet::kickQueue.try_pop(kickPlayerId))
		{
			IPlayer* player = SampVoiceComponent::GetPlayers()->get(kickPlayerId);
			if (player)
			{
				player->kick();
			}
		}
	}
}

bool OmpNet::SendRPC(const uint8_t rpcId, const uint16_t playerId, const void* const dataPtr, const int dataSize)
{
	auto bitStream = std::make_unique<BitStream>((uint8_t*)(dataPtr), dataSize, true);

	const std::shared_lock<std::shared_mutex> lock{ OmpNet::rpcQueueMutex };

	return OmpNet::rpcQueue.try_emplace(std::move(bitStream), playerId, rpcId);
}

bool OmpNet::SendPacket(const uint8_t packetId, const uint16_t playerId, const void* const dataPtr, const int dataSize)
{
	auto bitStream = std::make_unique<BitStream>(sizeof(packetId) + dataSize);

	bitStream->Write(packetId);
	bitStream->Write(static_cast<const char*>(dataPtr), dataSize);

	const std::shared_lock<std::shared_mutex> lock{ OmpNet::packetQueueMutex };

	return OmpNet::packetQueue.try_emplace(std::move(bitStream), playerId);
}

bool OmpNet::KickPlayer(const uint16_t playerId) noexcept
{
	const std::shared_lock<std::shared_mutex> lock{ OmpNet::kickQueueMutex };

	return OmpNet::kickQueue.try_emplace(playerId);
}

// ----------------------------------------------------------------------------

std::size_t OmpNet::AddConnectCallback(ConnectCallback callback) noexcept
{
	if (!OmpNet::initStatus) return -1;

	for (std::size_t i{ 0 }; i < OmpNet::connectCallbacks.size(); ++i)
	{
		if (OmpNet::connectCallbacks[i] == nullptr)
		{
			OmpNet::connectCallbacks[i] = std::move(callback);
			return i;
		}
	}

	OmpNet::connectCallbacks.emplace_back(std::move(callback));
	return OmpNet::connectCallbacks.size() - 1;
}

std::size_t OmpNet::AddPacketCallback(PacketCallback callback) noexcept
{
	if (!OmpNet::initStatus) return -1;

	for (std::size_t i{ 0 }; i < OmpNet::packetCallbacks.size(); ++i)
	{
		if (OmpNet::packetCallbacks[i] == nullptr)
		{
			OmpNet::packetCallbacks[i] = std::move(callback);
			return i;
		}
	}

	OmpNet::packetCallbacks.emplace_back(std::move(callback));
	return OmpNet::packetCallbacks.size() - 1;
}

std::size_t OmpNet::AddDisconnectCallback(DisconnectCallback callback) noexcept
{
	if (!OmpNet::initStatus) return -1;

	for (std::size_t i{ 0 }; i < OmpNet::disconnectCallbacks.size(); ++i)
	{
		if (OmpNet::disconnectCallbacks[i] == nullptr)
		{
			OmpNet::disconnectCallbacks[i] = std::move(callback);
			return i;
		}
	}

	OmpNet::disconnectCallbacks.emplace_back(std::move(callback));
	return OmpNet::disconnectCallbacks.size() - 1;
}

void OmpNet::RemoveConnectCallback(const std::size_t callback) noexcept
{
	if (!OmpNet::initStatus) return;

	if (callback >= OmpNet::connectCallbacks.size())
		return;

	OmpNet::connectCallbacks[callback] = nullptr;
}

void OmpNet::RemovePacketCallback(const std::size_t callback) noexcept
{
	if (!OmpNet::initStatus) return;

	if (callback >= OmpNet::packetCallbacks.size())
		return;

	OmpNet::packetCallbacks[callback] = nullptr;
}

void OmpNet::RemoveDisconnectCallback(const std::size_t callback) noexcept
{
	if (!OmpNet::initStatus) return;

	if (callback >= OmpNet::disconnectCallbacks.size())
		return;

	OmpNet::disconnectCallbacks[callback] = nullptr;
}

bool OmpNet::onReceivePacket(IPlayer& peer, int id, NetworkBitStream& bs)
{
	bool breakStatus{ true };

	for (const auto& packetCallback : OmpNet::packetCallbacks)
	{
		if (packetCallback != nullptr && !packetCallback(peer.getID(), bs))
			breakStatus = false;
	}

	if (breakStatus) return false;
	return true;
}

bool OmpNet::onReceiveRPC(IPlayer& peer, int id, NetworkBitStream& bs)
{
	if (id == int(SampVoiceComponent::NetworkID::RPC_PlayerConnect))
	{
		const auto sender = peer.getID();

		if (sender >= 0 && sender < PLAYER_POOL_SIZE)
		{
			if (OmpNet::playerStatus[sender])
			{
				for (const auto& disconnectCallback : OmpNet::disconnectCallbacks)
				{
					if (disconnectCallback != nullptr) disconnectCallback(sender);
				}
			}

			OmpNet::playerStatus[sender] = true;

			for (const auto& connectCallback : OmpNet::connectCallbacks)
			{
				if (connectCallback != nullptr) connectCallback(sender, bs);
			}
		}
	}
	return true;
}

void OmpNet::onPlayerDisconnect(IPlayer& player, PeerDisconnectReason reason)
{
	if (OmpNet::playerStatus[player.getID()])
	{
		for (const auto& disconnectCallback : OmpNet::disconnectCallbacks)
		{
			if (disconnectCallback != nullptr) disconnectCallback(player.getID());
		}
	}

	OmpNet::playerStatus[player.getID()] = false;
}
