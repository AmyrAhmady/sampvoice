/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "NetHandler.h"
#include "bitstream.hpp"
#include <random>

#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#endif

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

#include <util/logger.h>
#include <util/memory.hpp>

#ifdef _WIN32
#define GetNetError() WSAGetLastError()
#else
#define GetNetError() errno
#define closesocket(sock) close(sock)
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

static inline uint64_t MakeQword(uint32_t ldword, uint32_t rdword) noexcept
{
	return static_cast<uint64_t>(ldword) << 32 | static_cast<uint64_t>(rdword);
}

static inline uint32_t MakeBytesFromBits(uint32_t bits) noexcept
{
	return (bits >> 3) + (bits & 7 ? 1 : 0);
}

bool NetHandler::Init(ICore* ompCore) noexcept
{
	if (NetHandler::initStatus) return false;

	Logger::Log("[sv:dbg:network:init] : module initializing...");

	ompNet = new OmpNet();

	NetHandler::ompCore = ompCore;

	if (!ompNet->Init(ompCore))
	{
		Logger::Log("[sv:err:network:init] : failed to init raknet");
		return false;
	}

	ompNet->AddConnectCallback(NetHandler::ConnectHandler);
	ompNet->AddPacketCallback(NetHandler::PacketHandler);
	ompNet->AddDisconnectCallback(NetHandler::DisconnectHandler);

	Logger::Log("[sv:dbg:network:init] : module initialized");

	NetHandler::initStatus = true;

	return true;
}

void NetHandler::Free() noexcept
{
	if (!NetHandler::initStatus) return;

	Logger::Log("[sv:dbg:network:free] : module releasing...");

	if (NetHandler::bindStatus)
	{
		closesocket(NetHandler::socketHandle);

		NetHandler::socketHandle = NULL;
		NetHandler::serverPort = NULL;

#ifdef _WIN32
		WSACleanup();
#endif

		{
			const std::unique_lock<std::shared_mutex> lock{ NetHandler::playerKeyToPlayerIdTableMutex };
			NetHandler::playerKeyToPlayerIdTable.clear();
		}

		for (uint16_t iPlayerId{ 0 }; iPlayerId < PLAYER_POOL_SIZE; ++iPlayerId)
		{
			NetHandler::playerStatusTable[iPlayerId].store(false, std::memory_order_release);
			std::atomic_store(&NetHandler::playerAddrTable[iPlayerId], { nullptr });
		}

		while (!NetHandler::controlQueue.empty()) NetHandler::controlQueue.pop();
	}

	NetHandler::bindStatus = false;

	ompNet->Free();
	delete ompNet;

	NetHandler::connectCallbacks.clear();
	NetHandler::playerInitCallbacks.clear();
	NetHandler::disconnectCallbacks.clear();

	Logger::Log("[sv:dbg:network:free] : module released");

	NetHandler::initStatus = false;
}

bool NetHandler::Bind() noexcept
{
	if (!NetHandler::initStatus) return false;

	if (NetHandler::bindStatus) return true;
	if (!ompNet->IsLoaded()) return false;

#ifdef _WIN32
	if (const int error = WSAStartup(MAKEWORD(2, 2), &WSADATA()))
	{
		Logger::Log("[sv:err:network:bind] : wsastartup error (code:%d)", error);
		return false;
	}
#endif

	if ((NetHandler::socketHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		Logger::Log("[sv:err:network:bind] : socket error (code:%d)", GetNetError());
#ifdef _WIN32
		WSACleanup();
#endif
		return false;
	}

	{
		const auto sendBufferSize{ kSendBufferSize }, recvBufferSize{ kRecvBufferSize };

		if (setsockopt(NetHandler::socketHandle, SOL_SOCKET, SO_SNDBUF, (char*)(&sendBufferSize), sizeof(sendBufferSize)) == SOCKET_ERROR ||
			setsockopt(NetHandler::socketHandle, SOL_SOCKET, SO_RCVBUF, (char*)(&recvBufferSize), sizeof(recvBufferSize)) == SOCKET_ERROR)
		{
			Logger::Log("[sv:err:network:bind] : setsockopt error (code:%d)", GetNetError());
			closesocket(NetHandler::socketHandle);
#ifdef _WIN32
			WSACleanup();
#endif
			return false;
		}
	}

	{
		sockaddr_in bindAddr{};

		int _port = SampVoiceComponent::instance ? SampVoiceComponent::instance->GetSampVoiceConfigInt("sampvoice.port") : NULL;

		bindAddr.sin_family = AF_INET;
		bindAddr.sin_addr.s_addr = INADDR_ANY;

		if (_port == NULL)
		{
			bindAddr.sin_port = NULL;
		}
		else
		{
			bindAddr.sin_port = htons(uint16_t(_port));
		}

		if (bind(NetHandler::socketHandle, (sockaddr*)(&bindAddr), sizeof(bindAddr)) == SOCKET_ERROR)
		{
			Logger::Log("[sv:err:network:bind] : bind error (code:%d)", GetNetError());
			closesocket(NetHandler::socketHandle);
#ifdef _WIN32
			WSACleanup();
#endif
			return false;
		}
	}

	{
		sockaddr_in hostAddr{};
		socklen_t hostAddrLen{ sizeof(hostAddr) };

		if (getsockname(NetHandler::socketHandle, (sockaddr*)(&hostAddr), &hostAddrLen) == SOCKET_ERROR)
		{
			Logger::Log("[sv:err:network:bind] : getsockname error (code:%d)", GetNetError());
			closesocket(NetHandler::socketHandle);
#ifdef _WIN32
			WSACleanup();
#endif
			return false;
		}

		NetHandler::serverPort = ntohs(hostAddr.sin_port);
	}

	Logger::Log("[sv:dbg:network:bind] : voice server running on port %hu", NetHandler::serverPort);

	NetHandler::bindStatus = true;

	return true;
}

void NetHandler::Process() noexcept
{
	static Timer::time_t lastTime{ 0 };

	assert(SampVoiceComponent::instance != nullptr);
	assert(SampVoiceComponent::GetPlayers() != nullptr);

	if (!NetHandler::initStatus) return;

	ompNet->Process();

	if (!NetHandler::bindStatus) return;

	const auto curTime = Timer::Get();

	if (curTime - lastTime >= kKeepAliveInterval)
	{
		VoicePacket keepAlivePacket;

		keepAlivePacket.packet = SV::VoicePacketType::keepAlive;
		keepAlivePacket.length = NULL;
		keepAlivePacket.packid = NULL;
		keepAlivePacket.sender = NULL;
		keepAlivePacket.stream = NULL;
		keepAlivePacket.svrkey = NULL;
		keepAlivePacket.CalcHash();

		IPlayerPool* playerPool = SampVoiceComponent::GetPlayers();
		for (IPlayer* player : playerPool->entries())
		{
			if (!NetHandler::playerStatusTable[player->getID()].load(std::memory_order_acquire))
				continue;

			const auto playerAddr = std::atomic_load(&NetHandler::playerAddrTable[player->getID()]);
			if (playerAddr == nullptr) continue;

			sendto(NetHandler::socketHandle, reinterpret_cast<char*>(&keepAlivePacket), sizeof(keepAlivePacket),
				NULL, reinterpret_cast<sockaddr*>(playerAddr.get()), sizeof(*playerAddr));
		}

		lastTime = curTime;
	}
}

bool NetHandler::SendControlPacket(const uint16_t playerId, const ControlPacket& controlPacket)
{
	if (!NetHandler::initStatus) return false;

	return ompNet->SendPacket(kRaknetPacketId, playerId, &controlPacket, controlPacket.GetFullSize());
}

bool NetHandler::SendVoicePacket(const uint16_t playerId, const VoicePacket& voicePacket)
{
	if (!NetHandler::bindStatus) return false;

	if (!NetHandler::playerStatusTable[playerId].load(std::memory_order_acquire))
		return false;

	const auto playerAddr = std::atomic_load(&NetHandler::playerAddrTable[playerId]);
	if (playerAddr == nullptr) return false;

	return sendto(NetHandler::socketHandle, (char*)(&voicePacket), voicePacket.GetFullSize(),
		NULL, (sockaddr*)(playerAddr.get()), sizeof(*playerAddr)) == voicePacket.GetFullSize();
}

ControlPacketContainerPtr NetHandler::ReceiveControlPacket(uint16_t& sender) noexcept
{
	if (!NetHandler::initStatus) return nullptr;

	if (NetHandler::controlQueue.empty()) return nullptr;

	auto packetInfo = std::move(*NetHandler::controlQueue.front());
	NetHandler::controlQueue.pop();

	sender = packetInfo.sender;
	return std::move(packetInfo.packet);
}

VoicePacketContainerPtr NetHandler::ReceiveVoicePacket()
{
	if (!NetHandler::bindStatus)
		return nullptr;

	sockaddr_in playerAddr{};
	socklen_t addrLen{ sizeof(playerAddr) };
	char packetBuffer[kMaxVoicePacketSize];

	const auto length = recvfrom(NetHandler::socketHandle, packetBuffer,
		sizeof(packetBuffer), NULL, reinterpret_cast<sockaddr*>(&playerAddr), &addrLen);

	if (length < static_cast<decltype(length)>(sizeof(VoicePacket)))
		return nullptr;

	const auto voicePacketPtr = reinterpret_cast<VoicePacket*>(packetBuffer);
	if (!voicePacketPtr->CheckHeader()) return nullptr;

	const auto voicePacketSize = voicePacketPtr->GetFullSize();
	if (length != voicePacketSize) return nullptr;

	const auto playerKey = MakeQword(playerAddr.sin_addr.s_addr, voicePacketPtr->svrkey);

	uint16_t playerId{ SV::kNonePlayer };

	{
		const std::shared_lock<std::shared_mutex> lock{ NetHandler::playerKeyToPlayerIdTableMutex };

		const auto iter = NetHandler::playerKeyToPlayerIdTable.find(playerKey);
		if (iter == NetHandler::playerKeyToPlayerIdTable.end()) return nullptr;

		playerId = iter->second;
	}

	if (!NetHandler::playerStatusTable[playerId].load(std::memory_order_acquire))
		return nullptr;

	if (!std::atomic_load(&NetHandler::playerAddrTable[playerId]))
	{
		const auto playerAddrPtr = std::make_shared<sockaddr_in>(playerAddr);
		if (playerAddrPtr == nullptr) return nullptr;

		std::shared_ptr<sockaddr_in> expAddrPtr{ nullptr };
		if (std::atomic_compare_exchange_strong(&NetHandler::playerAddrTable[playerId], &expAddrPtr, playerAddrPtr))
		{
			Logger::Log("[sv:dbg:network:receive] : player (%hu) identified (port:%hu)", playerId, ntohs(playerAddr.sin_port));

			ControlPacket* controlPacket{ nullptr };
			PackAlloca(controlPacket, SV::ControlPacketType::pluginInit, sizeof(SV::PluginInitPacket));
			PackGetStruct(controlPacket, SV::PluginInitPacket)->bitrate = SV::kDefaultBitrate;
			PackGetStruct(controlPacket, SV::PluginInitPacket)->mute = false;

			for (const auto& playerInitCallback : NetHandler::playerInitCallbacks)
			{
				if (playerInitCallback != nullptr) playerInitCallback(playerId, *PackGetStruct(controlPacket, SV::PluginInitPacket));
			}

			if (!NetHandler::SendControlPacket(playerId, *controlPacket))
				Logger::Log("[sv:err:network:receive] : failed to send player (%hu) plugin init packet", playerId);
		}
	}

	if (voicePacketPtr->packet == SV::VoicePacketType::keepAlive)
		return nullptr;

	auto voicePacket = MakeVoicePacketContainer(voicePacketPtr, voicePacketSize);
	if (voicePacket == nullptr) return nullptr;

	auto& voicePacketRef = *voicePacket;

	voicePacketRef->sender = playerId;
	voicePacketRef->svrkey = NULL;

	return voicePacket;
}

std::size_t NetHandler::AddConnectCallback(ConnectCallback callback) noexcept
{
	if (!NetHandler::initStatus) return -1;

	for (std::size_t i{ 0 }; i < NetHandler::connectCallbacks.size(); ++i)
	{
		if (NetHandler::connectCallbacks[i] == nullptr)
		{
			NetHandler::connectCallbacks[i] = std::move(callback);
			return i;
		}
	}

	NetHandler::connectCallbacks.emplace_back(std::move(callback));
	return NetHandler::connectCallbacks.size() - 1;
}

std::size_t NetHandler::AddPlayerInitCallback(PlayerInitCallback callback) noexcept
{
	if (!NetHandler::initStatus) return -1;

	for (std::size_t i{ 0 }; i < NetHandler::playerInitCallbacks.size(); ++i)
	{
		if (NetHandler::playerInitCallbacks[i] == nullptr)
		{
			NetHandler::playerInitCallbacks[i] = std::move(callback);
			return i;
		}
	}

	NetHandler::playerInitCallbacks.emplace_back(std::move(callback));
	return NetHandler::playerInitCallbacks.size() - 1;
}

std::size_t NetHandler::AddDisconnectCallback(DisconnectCallback callback) noexcept
{
	if (!NetHandler::initStatus) return -1;

	for (std::size_t i{ 0 }; i < NetHandler::disconnectCallbacks.size(); ++i)
	{
		if (NetHandler::disconnectCallbacks[i] == nullptr)
		{
			NetHandler::disconnectCallbacks[i] = std::move(callback);
			return i;
		}
	}

	NetHandler::disconnectCallbacks.emplace_back(std::move(callback));
	return NetHandler::disconnectCallbacks.size() - 1;
}

void NetHandler::RemoveConnectCallback(const std::size_t callback) noexcept
{
	if (!NetHandler::initStatus) return;

	if (callback >= NetHandler::connectCallbacks.size())
		return;

	NetHandler::connectCallbacks[callback] = nullptr;
}

void NetHandler::RemovePlayerInitCallback(const std::size_t callback) noexcept
{
	if (!NetHandler::initStatus) return;

	if (callback >= NetHandler::playerInitCallbacks.size())
		return;

	NetHandler::playerInitCallbacks[callback] = nullptr;
}

void NetHandler::RemoveDisconnectCallback(const std::size_t callback) noexcept
{
	if (!NetHandler::initStatus) return;

	if (callback >= NetHandler::disconnectCallbacks.size())
		return;

	NetHandler::disconnectCallbacks[callback] = nullptr;
}

bool NetHandler::ConnectHandler(const uint16_t playerId, NetworkBitStream& bs)
{
	if (!NetHandler::initStatus) return true;

	const auto rpcParametersLength = MakeBytesFromBits(bs.GetNumberOfBitsUsed());
	const auto connectStruct = (SV::ConnectPacket*)(Memory::Scanner(bs.GetData(),
		rpcParametersLength).Find(SV::kSignaturePattern, SV::kSignatureMask));

	const uint8_t* connectStructEndPointer = ((uint8_t*)(connectStruct)) + sizeof(*connectStruct);
	const uint8_t* connectPacketEndPointer = bs.GetData() + rpcParametersLength;

	if (connectStruct == nullptr || connectStructEndPointer > connectPacketEndPointer)
		return true;

	IPlayer* player = SampVoiceComponent::GetPlayers()->get(playerId);
	if (player == nullptr)
		return true;

	const uint32_t playerAddr = player->getNetworkData().networkID.address.v4;
	if (playerAddr == NULL || playerAddr == UNASSIGNED_PLAYER_ID.binaryAddress) return true;

	if (NetHandler::playerStatusTable[playerId].exchange(false, std::memory_order_acq_rel))
	{
		for (const auto& disconnectCallback : NetHandler::disconnectCallbacks)
		{
			if (disconnectCallback != nullptr) disconnectCallback(playerId);
		}
	}

	Logger::Log("[sv:dbg:network:connect] : connecting player (%hu) with address (%s) ...",
		playerId, inet_ntoa(*(in_addr*)(&playerAddr)));

	std::mt19937 genRandomNumber(clock());
	uint32_t randomNumber; uint64_t playerKey;

	do playerKey = MakeQword(playerAddr, randomNumber = genRandomNumber());
	while (randomNumber == NULL || NetHandler::playerKeyToPlayerIdTable.find(playerKey) !=
		NetHandler::playerKeyToPlayerIdTable.end());

	Logger::Log("[sv:dbg:network:connect] : player (%hu) assigned key (%llx)", playerId, playerKey);

	std::atomic_store(&NetHandler::playerAddrTable[playerId], { nullptr });

	{
		const std::unique_lock<std::shared_mutex> lock{ NetHandler::playerKeyToPlayerIdTableMutex };

		NetHandler::playerKeyToPlayerIdTable.erase(NetHandler::playerKeyTable[playerId]);
		NetHandler::playerKeyToPlayerIdTable[playerKey] = playerId;
	}

	NetHandler::playerKeyTable[playerId] = playerKey;

	for (const auto& connectCallback : NetHandler::connectCallbacks)
	{
		if (connectCallback != nullptr) connectCallback(playerId, *connectStruct);
	}

	NetHandler::playerStatusTable[playerId].store(true, std::memory_order_release);

	ControlPacket* controlPacket{ nullptr };
	PackAlloca(controlPacket, SV::ControlPacketType::serverInfo, sizeof(SV::ServerInfoPacket));
	PackGetStruct(controlPacket, SV::ServerInfoPacket)->serverPort = NetHandler::serverPort;
	PackGetStruct(controlPacket, SV::ServerInfoPacket)->serverKey = randomNumber;
	if (!NetHandler::SendControlPacket(playerId, *controlPacket))
		Logger::Log("[sv:err:network:connect] : failed to send server info packet to player (%hu)", playerId);

	return true;
}

bool NetHandler::PacketHandler(const uint16_t playerId, NetworkBitStream& bs)
{
	if (!NetHandler::initStatus) return true;


	if (bs.GetNumberOfBytesUsed() < sizeof(uint8_t) + sizeof(ControlPacket)) return true;

	const auto controlPacketPtr = (ControlPacket*)(bs.GetData() + sizeof(uint8_t));
	const auto controlPacketSize = bs.GetNumberOfBytesUsed() - sizeof(uint8_t);

	if (controlPacketSize != controlPacketPtr->GetFullSize()) return false;

	NetHandler::controlQueue.try_emplace(MakeControlPacketContainer(controlPacketPtr, controlPacketSize), playerId);

	return false;
}

void NetHandler::DisconnectHandler(const uint16_t playerId)
{
	if (!NetHandler::initStatus) return;

	if (!NetHandler::playerStatusTable[playerId].exchange(false, std::memory_order_acq_rel))
		return;

	Logger::Log("[sv:dbg:network:connect] : disconnecting player (%hu) ...", playerId);

	std::atomic_store(&NetHandler::playerAddrTable[playerId], { nullptr });

	{
		const std::unique_lock<std::shared_mutex> lock{ NetHandler::playerKeyToPlayerIdTableMutex };
		NetHandler::playerKeyToPlayerIdTable.erase(NetHandler::playerKeyTable[playerId]);
	}

	NetHandler::playerKeyTable[playerId] = NULL;

	for (const auto& disconnectCallback : NetHandler::disconnectCallbacks)
	{
		if (disconnectCallback != nullptr) disconnectCallback(playerId);
	}
}

bool NetHandler::initStatus{ false };
bool NetHandler::bindStatus{ false };

OmpNet* NetHandler::ompNet = nullptr;
ICore* NetHandler::ompCore = nullptr;

SOCKET NetHandler::socketHandle{ NULL };
uint16_t NetHandler::serverPort{ NULL };

std::array<std::atomic_bool, PLAYER_POOL_SIZE> NetHandler::playerStatusTable{};
std::array<std::shared_ptr<sockaddr_in>, PLAYER_POOL_SIZE> NetHandler::playerAddrTable{};
std::array<uint64_t, PLAYER_POOL_SIZE> NetHandler::playerKeyTable{};

std::shared_mutex NetHandler::playerKeyToPlayerIdTableMutex;
std::map<uint64_t, uint16_t> NetHandler::playerKeyToPlayerIdTable;

std::vector<NetHandler::ConnectCallback> NetHandler::connectCallbacks;
std::vector<NetHandler::PlayerInitCallback> NetHandler::playerInitCallbacks;
std::vector<NetHandler::DisconnectCallback> NetHandler::disconnectCallbacks;

SPSCQueue<NetHandler::ControlPacketInfo> NetHandler::controlQueue{ 32 * PLAYER_POOL_SIZE };
