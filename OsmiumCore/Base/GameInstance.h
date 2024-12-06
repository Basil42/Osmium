//
// Created by nicolas.gerard on 2024-12-04.
//

#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H
#include <condition_variable>
#include <imgui.h>
#include <mutex>



class GameInstance {
    //syncing stuff
    std::mutex SimulationCompletionMutex;
    bool isSimOver;
    std::condition_variable SimulationConditionVariable;
    std::mutex renderDataMutex;
    std::condition_variable renderDataUpdateConditionVariable;
    bool isRenderUpdateOver;
    std::mutex ImguiMutex;
    bool isImguiNewFrameReady;
    bool isImguiUpdateOver;
    std::condition_variable ImguiUpdateConditionVariable;

    void RenderImGuiFrameTask();
    //temp
    bool showDemoWindow;
    bool showAnotherWindow;
    //Ecapsulate these two fields into a separate struct
    ImVec4 ImgGuiClearColor;
    ImGuiIO io;
    //we pass this through parameters so we can latert launch the game in editor context
    void GameLoop();
    void RenderLoop();

    void RenderDateUpdate();

public:

    void run();

};

struct gameConfig {

};

#endif //GAMEINSTANCE_H
