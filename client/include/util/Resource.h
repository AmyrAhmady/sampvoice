/*
    This is a SampVoice project file
    Author: CyberMor <cyber.mor.2020@gmail.ru>
    open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

    See more here https://github.com/AmyrAhmady/sampvoice
    Original repository: https://github.com/CyberMor/sampvoice

    Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <memory>

#include <Windows.h>

class Resource {

    Resource() = delete;
    Resource(const Resource&) = delete;
    Resource(Resource&&) = delete;
    Resource& operator=(const Resource&) = delete;
    Resource& operator=(Resource&&) = delete;

public:

    explicit Resource(HMODULE hModule, DWORD rId, LPCSTR rType);

    ~Resource() noexcept = default;

public:

    LPVOID GetDataPtr() const noexcept;
    DWORD GetDataSize() const noexcept;

private:

    LPVOID dataPtr { nullptr };
    DWORD dataSize { 0 };

};

using ResourcePtr = std::unique_ptr<Resource>;
#define MakeResource std::make_unique<Resource>
