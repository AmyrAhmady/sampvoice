/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <shared_mutex>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#ifndef _WIN32
#define SOCKET int
#endif

#include <SPSCQueue.h>
#include <util/ompnet.h>
#include <util/timer.h>
#include <map>

#include "ControlPacket.h"
#include "VoicePacket.h"
#include "Header.h"

class NetHandler {

	NetHandler() = delete;
	~NetHandler() = delete;
	NetHandler(const NetHandler&) = delete;
	NetHandler(NetHandler&&) = delete;
	NetHandler& operator=(const NetHandler&) = delete;
	NetHandler& operator=(NetHandler&&) = delete;

private:

	static constexpr uint8_t kRaknetPacketId = 222;
	static constexpr uint32_t kMaxVoicePacketSize = 1400;
	static constexpr uint32_t kMaxVoiceDataSize = kMaxVoicePacketSize - sizeof(VoicePacket);
	static constexpr uint32_t kSendBufferSize = 16 * 1024 * 1024;
	static constexpr uint32_t kRecvBufferSize = 32 * 1024 * 1024;
	static constexpr Timer::time_t kKeepAliveInterval = 10000;

private:

	using ConnectCallback = std::function<void(uint16_t, const SV::ConnectPacket&)>;
	using PlayerInitCallback = std::function<void(uint16_t, SV::PluginInitPacket&)>;
	using DisconnectCallback = std::function<void(uint16_t)>;

public:

	static bool Init(ICore* ompCore) noexcept;
	static void Free() noexcept;

	static bool Bind() noexcept;
	static void Process() noexcept;

	static OmpNet* GetOmpNet() { return ompNet; }

	static bool SendControlPacket(uint16_t playerId, const ControlPacket& controlPacket);
	static bool SendVoicePacket(uint16_t playerId, const VoicePacket& voicePacket);
	static ControlPacketContainerPtr ReceiveControlPacket(uint16_t& sender) noexcept;
	static VoicePacketContainerPtr ReceiveVoicePacket();

	static std::size_t AddConnectCallback(ConnectCallback callback) noexcept;
	static std::size_t AddPlayerInitCallback(PlayerInitCallback callback) noexcept;
	static std::size_t AddDisconnectCallback(DisconnectCallback callback) noexcept;
	static void RemoveConnectCallback(std::size_t callback) noexcept;
	static void RemovePlayerInitCallback(std::size_t callback) noexcept;
	static void RemoveDisconnectCallback(std::size_t callback) noexcept;

private:

	static bool ConnectHandler(uint16_t playerId, NetworkBitStream& bs);
	static bool PacketHandler(uint16_t playerId, NetworkBitStream& bs);
	static void DisconnectHandler(uint16_t playerId);

private:

	static bool initStatus;
	static bool bindStatus;

	static OmpNet* ompNet;
	static ICore* ompCore;

	static SOCKET socketHandle;
	static uint16_t serverPort;

	static std::vector<ConnectCallback> connectCallbacks;
	static std::vector<PlayerInitCallback> playerInitCallbacks;
	static std::vector<DisconnectCallback> disconnectCallbacks;

	static std::array<std::atomic_bool, PLAYER_POOL_SIZE> playerStatusTable;
	static std::array<std::shared_ptr<sockaddr_in>, PLAYER_POOL_SIZE> playerAddrTable;
	static std::array<uint64_t, PLAYER_POOL_SIZE> playerKeyTable;

	static std::shared_mutex playerKeyToPlayerIdTableMutex;
	static std::map<uint64_t, uint16_t> playerKeyToPlayerIdTable;

private:

	struct ControlPacketInfo {

		ControlPacketInfo() noexcept = default;
		ControlPacketInfo(const ControlPacketInfo&) = default;
		ControlPacketInfo(ControlPacketInfo&&) noexcept = default;
		ControlPacketInfo& operator=(const ControlPacketInfo&) = default;
		ControlPacketInfo& operator=(ControlPacketInfo&&) noexcept = default;

	private:

		using PacketPtr = ControlPacketContainerPtr;

	public:

		explicit ControlPacketInfo(PacketPtr packet, uint16_t sender) noexcept
			: packet(std::move(packet)), sender(sender) {}

		~ControlPacketInfo() noexcept = default;

	public:

		PacketPtr packet{ nullptr };
		uint16_t sender{ NULL };

	};

	static SPSCQueue<ControlPacketInfo> controlQueue;

};
