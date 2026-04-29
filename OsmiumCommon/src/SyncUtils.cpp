//
// Created by nicol on 2026-04-29.
//
#include "SyncUtils.h"


void Sync::DependencySignal::WaitForSignalAndRearm() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this]() { return signaled; });
    signaled = false;
}

void Sync::DependencySignal::Signal() {
    signaled = !signaled;
    cv.notify_one();//there should never be several consumer for a given signal
}
