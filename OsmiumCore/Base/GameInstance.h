//
// Created by nicolas.gerard on 2024-12-04.
//

#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H
#include <condition_variable>
#include <functional>
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
    std::span<Sync::DependencySignal>& m_GameLoopExternalProviders;//implicitly the render data copy but they technically cannot run in parrallel, the editor can be
    std::span<Sync::DependencySignal>& m_GameLoopExternalConsumers;//also the render date copy and editor

    Sync::DependencySignal m_RenderDataProvidersSync {
        .requiredProduts = 2,//render and tick
    };
    Sync::DependencySignal m_TickProvidersSync {
        .requiredProduts = 1,//only render data update, the editor is an external provider/consumer, both the tick and render wait on its signal
    };
    Sync::DependencySignal m_RenderProvidersSync {
        .requiredProduts = 1,//renderData
    };
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

    void run(const std::string &appName, std::span<Sync::DependencySignal>& producers,std::span<Sync::DependencySignal>& consumers);

    void SetMainCamera(GameObjectHandle editor_camera);

    void SetMainCamera(GOC_Camera *camComp);

    void SetDirectionalLight(GOC_DirectionalLight * DirLightComp);

    static glm::mat4 getMainCameraViewMatrix();
};

struct gameConfig {

};

#endif //GAMEINSTANCE_H
