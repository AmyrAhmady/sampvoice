/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <memory>
#include <cstdint>
#include <functional>
#include <shared_mutex>
#include <vector>
#include <array>
#include "sdk.hpp"
#include <MPMCQueue.h>
#include <raknet/bitstream.h>
#include <raknet/networktypes.h>

#include "memory.hpp"

class OmpNet: public PlayerConnectEventHandler, public NetworkInEventHandler, public NetworkOutEventHandler {

private:

	using ConnectCallback = std::function<void(uint16_t, NetworkBitStream&)>;
	using PacketCallback = std::function<bool(uint16_t, NetworkBitStream&)>;
	using DisconnectCallback = std::function<void(uint16_t)>;

public:
	OmpNet()
	{}

	~OmpNet()
	{}

	bool Init(ICore* core) noexcept;
	void Free() noexcept;

	bool IsLoaded() noexcept;
	void Process() noexcept;

	bool SendRPC(uint8_t rpcId, uint16_t playerId, const void* dataPtr, int dataSize);
	bool SendPacket(uint8_t packetId, uint16_t playerId, const void* dataPtr, int dataSize);
	bool KickPlayer(uint16_t playerId) noexcept;

	bool onReceivePacket(IPlayer& peer, int id, NetworkBitStream& bs) override;
	bool onReceiveRPC(IPlayer& peer, int id, NetworkBitStream& bs) override;
	void onPlayerDisconnect(IPlayer& player, PeerDisconnectReason reason) override;

	std::size_t AddConnectCallback(ConnectCallback callback) noexcept;
	std::size_t AddPacketCallback(PacketCallback callback) noexcept;
	std::size_t AddDisconnectCallback(DisconnectCallback callback) noexcept;
	void RemoveConnectCallback(std::size_t callback) noexcept;
	void RemovePacketCallback(std::size_t callback) noexcept;
	void RemoveDisconnectCallback(std::size_t callback) noexcept;

public:
	static OmpNet* instance;
	ICore* ompCore = nullptr;

private:

	bool initStatus{ false };
	bool loadStatus{ false };

	std::vector<ConnectCallback> connectCallbacks;
	std::vector<PacketCallback> packetCallbacks;
	std::vector<DisconnectCallback> disconnectCallbacks;

	std::array<bool, PLAYER_POOL_SIZE> playerStatus;

	RPCFunction origConnectHandler;

private:

	struct SendRpcInfo {

		SendRpcInfo() noexcept = default;
		SendRpcInfo(const SendRpcInfo&) = default;
		SendRpcInfo(SendRpcInfo&&) noexcept = default;
		SendRpcInfo& operator=(const SendRpcInfo&) = default;
		SendRpcInfo& operator=(SendRpcInfo&&) noexcept = default;

	private:

		using BitStreamPtr = std::unique_ptr<BitStream>;

	public:

		explicit SendRpcInfo(BitStreamPtr bitStream, uint16_t playerId, uint8_t rpcId) noexcept
			: bitStream(std::move(bitStream)), playerId(playerId), rpcId(rpcId) {}

		~SendRpcInfo() noexcept = default;

	public:

		BitStreamPtr bitStream{ nullptr };
		uint16_t playerId{ NULL };
		uint8_t rpcId{ NULL };

	};

	struct SendPacketInfo {

		SendPacketInfo() noexcept = default;
		SendPacketInfo(const SendPacketInfo&) = default;
		SendPacketInfo(SendPacketInfo&&) noexcept = default;
		SendPacketInfo& operator=(const SendPacketInfo&) = default;
		SendPacketInfo& operator=(SendPacketInfo&&) noexcept = default;

	private:

		using BitStreamPtr = std::unique_ptr<BitStream>;

	public:

		explicit SendPacketInfo(BitStreamPtr bitStream, uint16_t playerId) noexcept
			: bitStream(std::move(bitStream)), playerId(playerId) {}

		~SendPacketInfo() noexcept = default;

	public:

		BitStreamPtr bitStream{ nullptr };
		uint16_t playerId{ NULL };

	};

	std::shared_mutex rpcQueueMutex;
	MPMCQueue<SendRpcInfo> rpcQueue{ 16 * PLAYER_POOL_SIZE };

	std::shared_mutex packetQueueMutex;
	MPMCQueue<SendPacketInfo> packetQueue{ 16 * PLAYER_POOL_SIZE };

	std::shared_mutex kickQueueMutex;
	MPMCQueue<uint16_t> kickQueue{ PLAYER_POOL_SIZE };

};
