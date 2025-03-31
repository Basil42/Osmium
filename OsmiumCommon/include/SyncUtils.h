//
// Created by nicolas.gerard on 2025-03-31.
//

#ifndef SYNCUTILS_H
#define SYNCUTILS_H
#include <mutex>
#include <condition_variable>
namespace Sync {
    struct SyncBoolCondition {
        std::mutex& mutex;
        bool& boolean;
        std::condition_variable& cv;
        SyncBoolCondition(std::mutex &mut, bool &b, std::condition_variable &cv) : mutex(mut), boolean(b), cv(cv) {}

        void waitAndLock() const {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock,[&](){return boolean;});
        }
    };
}
#endif //SYNCUTILS_H
