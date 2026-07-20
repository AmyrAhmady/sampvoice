/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#pragma once

#include <set>
#include <mutex>

#include "Stream.h"

class StreamManager {
public:
    static void RegisterStream(Stream* stream);
    static void UnregisterStream(Stream* stream);
    static bool IsValidStream(Stream* stream);

private:
    static std::set<Stream*> streams_;
    static std::mutex mutex_;
};