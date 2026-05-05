//
// Created by nicolas.gerard on 2025-03-31.
//

#ifndef SYNCUTILS_H
#define SYNCUTILS_H
#include <mutex>
#include <condition_variable>
namespace Sync {
    //a struct that is used to sync pairs of dependent processes
    //If each process needs to run once they can wait for a signal at the top of the process and signal the other of the pair.
    //Each process needs wait for a seperate value of signaled (on waits for true , the other for false)
    struct DependencySignal {
        std::mutex mutex;
        unsigned int products = 0;
        unsigned int requiredProduts = 1;
        std::condition_variable cv;

        void WaitForProductsAndRearm();
        void SignalProductComplete();
    };
}
#endif //SYNCUTILS_H
