//
// Created by nicolas.gerard on 2024-12-04.
//

#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <span>
#include <vector>
#include <glm/fwd.hpp>

#include "../../OsmiumCommon/include/CommonConfig.h"
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
    bool simShouldShutoff = false;

    uint64_t m_TickFrameCounter=0;
    uint64_t m_RenderUpdateFrameCounter=0;
    ResourceArray<GameObject,MAX_GAMEOBJECTS>*GameObjects = nullptr;
    std::mutex GOcreationQueueMutex;
    std::queue<std::pair<GameObjectCreateInfo,std::function<void(GameObject*)>>> gameObjectsCreationQueue;
    // std::condition_variable GOcreationQueueConditionVariable;
    std::mutex GOdestructionQueueMutex;
    std::queue<GameObjectHandle> gameObjectsDestructionQueue;
    // std::condition_variable GOdestructionQueueConditionVariable;
    GOC_Camera* mainCamera = nullptr;//stable for game objects, should probably use some kind of handle though
    GOC_DirectionalLight * directionLight = nullptr;

    std::mutex GOOperationQueueMutex;
    // std::condition_variable GOOperationQueueConditionVariable;
    std::queue<std::pair<GameObject*,std::function<void(GameObject*)>>> gameobjectsThreadsafeOperationQueue;//used to queue function from outside the sim in the thread safe fashion

    static void LoadingRoutine();

    static void UnloadingRoutine();
    //we pass this through parameters so we can latert launch the game in editor context
    void GameTick();

    void RenderDataUpdate();

public:
    explicit GameInstance(const std::string &appName = "Osmium");
    void CreateNewGameObject(GameObjectCreateInfo &createStruct, const std::function<void(GameObject *)>& callback = nullptr);//The object itself will be available next simulation tick
    void DestroyGameObject(GameObject * gameObject);
    [[nodiscard]] ResourceArray<GameObject,MAX_GAMEOBJECTS>& GetGameObjects() const;

    void AddGameObjectOperation(GameObject* gameObject, const std::function<void(GameObject*)>& operation);//used to do run game object related logic from non sim thread (like the editor)

    void run();

    void SetMainCamera(GameObjectHandle editor_camera);

    void SetMainCamera(GOC_Camera *camComp);

    void SetDirectionalLight(GOC_DirectionalLight * DirLightComp);

    static glm::mat4 getMainCameraViewMatrix();
};

struct gameConfig {

};

#endif //GAMEINSTANCE_H
