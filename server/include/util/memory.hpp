/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <memory>
#include <type_traits>
#include <iostream>
#include <cassert>
#include <cstring>
#include <fstream>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif

#ifdef _WIN32
#define STDCALL __stdcall
#define THISCALL __thiscall
#else
#define STDCALL
#define THISCALL
#endif

#define RequireArithmeticType(type) static_assert(std::is_arithmetic<type>::value, #type " is not arithmetic type")
#define RequireAddressType(type) static_assert(std::is_arithmetic<type>::value || std::is_pointer<type>::value, #type " is not address type")
#define RequireVarHasType(var, type) static_assert(std::is_same<decltype(var), type>::value, #var " type does not correspond to " #type)
#define SizeOfArray(arr) ((sizeof(arr) / sizeof(0[arr])) / ((size_t)(!(sizeof(arr) % sizeof(0[arr])))))

namespace Memory
{
	using addr_t = void*;
	using size_t = std::size_t;
	using byte_t = unsigned char;

	template<class ObjectType> class ObjectContainer {
	public:

		ObjectContainer() = default;
		ObjectContainer(const ObjectContainer&) = default;
		ObjectContainer(ObjectContainer&&) noexcept = default;
		ObjectContainer& operator=(const ObjectContainer&) = default;
		ObjectContainer& operator=(ObjectContainer&&) noexcept = default;

	public:

		explicit ObjectContainer(const size_t addMemSize)
			: bytes(sizeof(ObjectType) + addMemSize) {}

		template<class MemAddrType = addr_t, class MemSizeType = size_t>
		explicit ObjectContainer(const MemAddrType memAddr, const MemSizeType memSize)
			: bytes((size_t)(memSize))
		{
			RequireAddressType(MemAddrType);
			RequireArithmeticType(MemSizeType);

			assert((addr_t)(memAddr));
			assert((size_t)(memSize));

			assert((size_t)(memSize) >= sizeof(ObjectType));

			std::memcpy(this->bytes.data(), (addr_t)(memAddr), this->bytes.size());
		}

		~ObjectContainer() noexcept = default;

	public:

		const ObjectType* operator->() const noexcept
		{
			return reinterpret_cast<const ObjectType*>(this->bytes.data());
		}

		ObjectType* operator->() noexcept
		{
			return reinterpret_cast<ObjectType*>(this->bytes.data());
		}

		const ObjectType* operator&() const noexcept
		{
			return reinterpret_cast<const ObjectType*>(this->bytes.data());
		}

		ObjectType* operator&() noexcept
		{
			return reinterpret_cast<ObjectType*>(this->bytes.data());
		}

		const addr_t GetData() const noexcept
		{
			return reinterpret_cast<const addr_t>(this->bytes.data());
		}

		addr_t GetData() noexcept
		{
			return reinterpret_cast<addr_t>(this->bytes.data());
		}

		size_t GetSize() const noexcept
		{
			return static_cast<size_t>(this->bytes.size());
		}

	private:

		std::vector<byte_t> bytes{ sizeof(ObjectType) };

	};

	template<class ObjectType> using ObjectContainerPtr = std::unique_ptr<ObjectContainer<ObjectType>>;

	class Scanner {
	public:

		Scanner() noexcept = default;
		Scanner(const Scanner&) noexcept = default;
		Scanner(Scanner&&) noexcept = default;
		Scanner& operator=(const Scanner&) noexcept = default;
		Scanner& operator=(Scanner&&) noexcept = default;

	public:

		template<class MemAddrType = addr_t, class MemSizeType = size_t>
		explicit Scanner(const MemAddrType memAddr, const MemSizeType memSize) noexcept
			: memAddr((addr_t)(memAddr)), memSize((size_t)(memSize))
		{
			RequireAddressType(MemAddrType);
			RequireArithmeticType(MemSizeType);

			assert((addr_t)(memAddr));
			assert((size_t)(memSize));
		}

		~Scanner() noexcept = default;

	public:

		addr_t Find(const char* const pattern, const char* const mask) const noexcept
		{
			if (!this->memAddr || !this->memSize) return nullptr;

			const char* currentByte = static_cast<char*>(this->memAddr);
			const char* const lastByte = reinterpret_cast<char*>((size_t)(this->memAddr)
				+ this->memSize - std::strlen(mask));

			for (size_t i; currentByte < lastByte; ++currentByte)
			{
				for (i = 0; mask[i]; ++i) if (mask[i] == 'x' &&
					pattern[i] != currentByte[i]) break;

				if (!mask[i]) break;
			}

			return currentByte != lastByte ? (addr_t)(currentByte) : nullptr;
		}

	private:

		addr_t memAddr{ nullptr };
		size_t memSize{ NULL };

	};

	using ScannerPtr = std::unique_ptr<Scanner>;

	template<class MemAddrType = addr_t, class ModuleAddrType = addr_t, class ModuleSizeType = size_t>
	static bool GetModuleInfo(const MemAddrType memAddr, ModuleAddrType& moduleAddr, ModuleSizeType& moduleSize) noexcept
	{
		RequireAddressType(MemAddrType);
		RequireAddressType(ModuleAddrType);
		RequireArithmeticType(ModuleSizeType);

#ifdef _WIN32
		MEMORY_BASIC_INFORMATION info{};

		if (VirtualQuery((LPCVOID)(memAddr), &info, sizeof(info)) == 0)
			return false;

		if ((moduleAddr = (ModuleAddrType)(info.AllocationBase)) == nullptr)
			return false;

		const auto dos = (IMAGE_DOS_HEADER*)(info.AllocationBase);
		const auto pe = (IMAGE_NT_HEADERS*)(((DWORD)(dos)) + dos->e_lfanew);

		if (pe->Signature != IMAGE_NT_SIGNATURE)
			return false;

		if ((moduleSize = (ModuleSizeType)(pe->OptionalHeader.SizeOfImage)) == 0)
			return false;
#else
		Dl_info info{};
		struct stat buf {};

		if (dladdr((addr_t)(memAddr), &info) == 0)
			return false;

		if (stat(info.dli_fname, &buf) == -1)
			return false;

		if ((moduleAddr = (ModuleAddrType)(info.dli_fbase)) == nullptr)
			return false;

		if ((moduleSize = (ModuleSizeType)(buf.st_size)) == 0)
			return false;
#endif

		return true;
	}

	static inline float qsqrt(const float number) noexcept
	{
		float result;

#ifdef _WIN32
		__asm {
			mov eax, number
			sub eax, 0x3f800000
			sar eax, 1
			add eax, 0x3f800000
			mov result, eax
		}
#else
		__asm__ __volatile__(
			"subl $0x3f800000, %1\n\t"
			"sarl $1, %1\n\t"
			"addl $0x3f800000, %1"
			: "=a"(result)
			: "a"(number)
		);
#endif

		return result;
	}
}

#define MakeObjectContainer(ObjectType) std::make_unique<Memory::ObjectContainer<ObjectType>>
#define MakeScanner                     std::make_unique<Memory::Scanner>
