//
// Created by nicolas.gerard on 2025-03-31.
//

#ifndef SYNCUTILS_H
#define SYNCUTILS_H
#include <array>
#include <mutex>
#include <condition_variable>
namespace Sync {
    //a struct that is used to sync pairs of dependent processes
    //If each process needs to run once they can wait for a signal at the top of the process and signal the other of the pair.
    //Each process needs wait for a seperate value of signaled (on waits for true , the other for false)
//each stage should only be signaled by one producer
    enum SyncStageComplete {
        SYNC_STAGE_TICK,
        SYNC_STAGE_RENDER,
        SYNC_STAGE_RENDER_UPDATE,
        SYNC_STAGE_RENDER_IMGUI_FRAME_START,
        SYNC_STAGE_RENDER_IMGUI_FRAME_END,
        SYNC_STAGE_EDITOR,
        SYNC_STAGE_MAX_VALUE
    };
    class SynchronizationManager {
        struct CompletionSignal {
            std::mutex mutex;
            uint64_t value = 0;
            std::condition_variable cv;
        };
        static std::array<CompletionSignal,SYNC_STAGE_MAX_VALUE> SyncPoints;
        public:

        static uint64_t Signal(SyncStageComplete stage);
        static void Wait(SyncStageComplete stage,uint_fast64_t frame);
    };

}
#endif //SYNCUTILS_H
