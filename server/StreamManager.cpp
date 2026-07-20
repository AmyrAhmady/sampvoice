/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "StreamManager.h"

void StreamManager::RegisterStream(Stream* stream) {
    std::lock_guard<std::mutex> lock(mutex_);
    streams_.insert(stream);
}

void StreamManager::UnregisterStream(Stream* stream) {
    std::lock_guard<std::mutex> lock(mutex_);
    streams_.erase(stream);
}

bool StreamManager::IsValidStream(Stream* stream) {
    std::lock_guard<std::mutex> lock(mutex_);
    return streams_.find(stream) != streams_.end();
}

std::set<Stream*> StreamManager::streams_;
std::mutex StreamManager::mutex_;