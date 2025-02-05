//
// Created by nicolas.gerard on 2024-12-04.
//

#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H
#include <condition_variable>
#include <functional>
#include <imgui.h>
#include <mutex>
#include <queue>
#include <vector>
#include <glm/fwd.hpp>

#include "config.h"
#include "GameObject.h"
#include "GameObjectCreation.h"
template <typename T,size_t Max_Capacity>class ResourceArray;
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

    ResourceArray<GameObject,MAX_GAMEOBJECTS>*GameObjects = nullptr;
    std::vector<GameObject> gameObjects;//Problem -> I need handles for these, as passing pointers around is dangerous
    std::queue<std::pair<GameObjectCreateInfo,std::function<void(GameObject*)>>> gameObjectsCreationQueue;
    std::queue<GameObjectHandle> gameObjectsDestructionQueue;
    GOC_Camera* mainCamera = nullptr;
    static GameInstance * instance;

    void CreateNewGameObject(GameObjectCreateInfo &createStruct, const std::function<void(GameObject *)>& callback = nullptr);//The object itself will be available next simulation tick
    void DestroyGameObject(GameObject * gameObject);

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
