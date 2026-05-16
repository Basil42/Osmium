//
// Created by nicol on 2026-04-29.
//
#include "SyncUtils.h"

#include <iostream>


std::array<Sync::SynchronizationManager::CompletionSignal,Sync::SYNC_STAGE_MAX_VALUE> Sync::SynchronizationManager::SyncPoints{};
uint_fast64_t Sync::SynchronizationManager::Signal(const SyncStageComplete stage) {
    auto& sync = SyncPoints[stage];
    std::unique_lock lock(sync.mutex);
    const uint64_t newCount = ++SyncPoints[stage].value;
    lock.unlock();
    sync.cv.notify_all();
    return newCount;
}

void Sync::SynchronizationManager::Wait(SyncStageComplete stage, uint_fast64_t frame) {
    std::unique_lock lock(SyncPoints[stage].mutex);
    SyncPoints[stage].cv.wait(lock, [stage, frame]()->bool {return SyncPoints[stage].value >frame;});//ensures the point needs to be signaled at least once
}
