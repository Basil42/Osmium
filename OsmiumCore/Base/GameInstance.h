//
// Created by nicolas.gerard on 2024-12-04.
//

#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H
#include <condition_variable>
#include <imgui.h>
#include <mutex>
#include <vector>
#include <glm/fwd.hpp>

#include "GameObject.h"


class GOC_Camera;

class GameInstance {
    //syncing stuff
    std::mutex SimulationCompletionMutex;
    bool isSimOver;
    std::condition_variable SimulationConditionVariable;
    std::mutex renderDataMutex;
    std::condition_variable renderDataUpdateConditionVariable;
    bool isRenderUpdateOver;
    std::mutex ImguiMutex;
    std::condition_variable ImguiNewFrameConditionVariable;
    bool isImguiNewFrameReady;
    bool isImguiUpdateOver = true;//should skip over that update on the first sim tick
    std::condition_variable ImguiUpdateConditionVariable;
    bool ImGuiShouldShutoff;
    bool simShouldShutoff;

    std::vector<GameObject> gameObjects;
    GOC_Camera* mainCamera;
    static GameInstance * instance;

    GameObject * CreateNewGameObject();

    void RenderImGuiFrameTask();
    void LoadingRoutine();
    //temp
    bool showDemoWindow;
    bool showAnotherWindow;
    //Ecapsulate these two fields into a separate struct
    ImVec4 ImgGuiClearColor;
    ImGuiIO io;
    //we pass this through parameters so we can latert launch the game in editor context
    void GameLoop();
    void RenderLoop();

    void RenderDataUpdate();

public:

    void run();

    static glm::mat4 getMainCameraViewMatrix();
};

struct gameConfig {

};

#endif //GAMEINSTANCE_H
