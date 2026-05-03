//
// Created by nicol on 2026-04-29.
//
#include "SyncUtils.h"


void Sync::DependencySignal::WaitForProductsAndRearm() {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this]() { return products >= requiredProduts; });
    products = 0;
}

void Sync::DependencySignal::SignalProductComplete() {
    std::unique_lock<std::mutex> lock(mutex);
    products++;
    lock.unlock();
    cv.notify_all();//there should never be several consumer for a given signal
}
