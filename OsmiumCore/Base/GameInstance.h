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
#include "SyncUtils.h"
#include "GOComponents/GOC_DirectionalLight.h"

struct ImGuiSyncStruct {
    std::mutex* imGuiMutex;
    std::condition_variable* imGuiNewFrameConditionVariable;
    bool* isImguiNewFrameReady;
    bool* ImGuiShouldShutoff;
};

template <typename T,size_t Max_Capacity>class ResourceArray;
class GOC_Camera;
class GameInstance {
    //syncing stuff
    Sync::DependencySignal SimulationSync;
    std::vector<Sync::DependencySignal*> m_GameLoopProviders{};//implicitly the render data copy but they technically cannot run in parrallel, the editor can be
    std::vector<Sync::DependencySignal> m_GameLoopConsumers;//also the render date copy and editor

    bool simShouldShutoff = false;

    ResourceArray<GameObject,MAX_GAMEOBJECTS>*GameObjects = nullptr;
    std::mutex creationQueueMutex;
    std::queue<std::pair<GameObjectCreateInfo,std::function<void(GameObject*)>>> gameObjectsCreationQueue;
    std::condition_variable creationQueueConditionVariable;
    std::mutex destructionQueueMutex;
    std::queue<GameObjectHandle> gameObjectsDestructionQueue;
    std::condition_variable destructionQueueConditionVariable;
    GOC_Camera* mainCamera = nullptr;//stable for game objects, should probably use some kind of handle though
    GOC_DirectionalLight * directionLight = nullptr;
    static GameInstance * instance;


    void LoadingRoutine();
    void UnloadingRoutine();
    //we pass this through parameters so we can latert launch the game in editor context
    void GameLoop();
    void RenderLoop();

    void RenderDataUpdate();

public:
    void CreateNewGameObject(GameObjectCreateInfo &createStruct, const std::function<void(GameObject *)>& callback = nullptr);//The object itself will be available next simulation tick
    void DestroyGameObject(GameObject * gameObject);
    [[nodiscard]] ResourceArray<GameObject,MAX_GAMEOBJECTS>& GetGameObjects() const;

    void run(const std::string &appName);

    void getGameLoopSyncStruct(Sync::DependencySignal* syncData);//can be used by the GL and editor to be signaled that the simulation step is over

    void SetMainCamera(GameObjectHandle editor_camera);

    void SetMainCamera(GOC_Camera *camComp);

    void SetDirectionalLight(GOC_DirectionalLight * DirLightComp);

    static glm::mat4 getMainCameraViewMatrix();
};

struct gameConfig {

};

#endif //GAMEINSTANCE_H
