//
// Created by nicolas.gerard on 2024-12-04.
//

#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H
#include <condition_variable>
#include <imgui.h>
#include <mutex>



class GameInstance {
    void RenderImGuiFrameTask();
    //temp
    bool showDemoWindow;
    bool showAnotherWindow;
    //Ecapsulate these two fields into a separate struct
    ImVec4 ImgGuiClearColor;
    ImGuiIO io;
    //we pass this through parameters so we can latert launch the game in editor context
    void GameLoop(std::mutex &SimulationMutex,bool &isSimOver,
    std::condition_variable &SimulationConditionVariable,
    std::mutex &renderDataUpdateMutex, bool &isRenderDataUpdateOver,
    std::condition_variable &renderDataUpdateConditionVariable,
    std::mutex &ImguiMutex, bool &isImguiUpdateOver,
    std::condition_variable &ImguiUpdateConditionVariable);

    void RenderLoop(std::mutex &renderMutex,bool &isRenderOver,
        std::condition_variable &renderConditionVariable,
        std::mutex &RenderDataUpdateMutex,bool &isRenderDataUpdateOver,
        std::condition_variable &RenderDataUpdateConditionVariable,
        std::mutex &ImGuiUpdateMutex,bool &isImGuiUpdateComplete,
        std::condition_variable &ImGuiUpdateConditionVariable);
public:
    

    void run();

};

struct gameConfig {

};

#endif //GAMEINSTANCE_H
