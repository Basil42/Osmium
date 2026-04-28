//
// Created by nicolas.gerard on 2025-03-31.
//

#ifndef SYNCUTILS_H
#define SYNCUTILS_H
#include <mutex>
#include <condition_variable>
namespace Sync {
    struct SyncCondition {
        std::mutex mutex;
        bool signaled = false;
        std::condition_variable cv;
    };
}
#endif //SYNCUTILS_H
