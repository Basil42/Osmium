//
// Created by nicolas.gerard on 2024-12-04.
//

#include "GameInstance.h"

#include <thread>

#include "OsmiumGL_API.h"
#include "../AssetManagement/AssetManager.h"
#include "../AssetManagement/AssetType/MeshAsset.h"
#include "../GOComponents/GOC_MeshRenderer.h"
#include "../GOComponents/GOC_Transform.h"
#include "../GOComponents/GOC_Camera.h"
#include "ResourceArray.h"
#include "GOComponents/GOC_PointLight.h"

GameInstance* GameInstance::instance = nullptr;//TODO refactor camera access to get rid of this
void GameInstance::GameLoop() {
    //double lastFrameTime = glfwGetTime();
    while (!OsmiumGL::ShouldClose()) {//might be thread unsafe to check this


        for (auto& provider : m_GameLoopExternalProviders) {
            provider.WaitForProductsAndRearm();
        }
        m_TickProvidersSync.WaitForProductsAndRearm();
        //note: the dependency mutexes are not locked from this point on.


        AssetManager::ProcessCallbacks();
        //game object creation
        while (!gameObjectsCreationQueue.empty()) {
            auto entry = gameObjectsCreationQueue.front();
            gameObjectsCreationQueue.pop();
            GameObject* newObj;
            const auto handle = GameObjects->emplace_new(newObj);
            newObj->Name = entry.first.name;
            newObj->Handle = handle;
            if (entry.second)entry.second(newObj);
        }

        //Do simulation things
        for (auto& game_object : *GameObjects) {
            game_object.UpdateComponents();
        }

        while (!gameObjectsDestructionQueue.empty()) {
            const GameObjectHandle obj = gameObjectsDestructionQueue.front();
            gameObjectsDestructionQueue.pop();
            GameObjects->Remove(obj);//This should be the only call to remove on this container
        }
        //notify consumers
        for (auto& consumer : m_GameLoopExternalConsumers) {
            consumer.SignalProductComplete();
        }
        m_RenderDataProvidersSync.SignalProductComplete();
    }
}

void GameInstance::RenderDataUpdate() {
    if (mainCamera == nullptr) return;
    //this is quite janky and annoying to extend
    mainCamera->RenderUpdate();

    directionLight->RenderUpdate();
    GOC_MeshRenderer::GORenderUpdate();
    GOC_PointLight::GORenderUpdate();

}

void GameInstance::run(const std::string &appName, std::span<Sync::DependencySignal>& producers,std::span<Sync::DependencySignal>& consumers) {

    instance = this;
    GameObjects = new ResourceArray<GameObject,MAX_GAMEOBJECTS>();
    OsmiumGL::Init(appName);
    m_GameLoopExternalProviders = producers;
    m_GameLoopExternalConsumers = consumers;
    //load the initial assets, probably in its own thread
    AssetManager::LoadAssetDatabase();
    auto SimulationThread = std::thread(&GameInstance::GameLoop,this);
    // auto ImGuiThread = std::thread(RenderImGuiFrameTask,this);
    auto LoadingThread = std::thread(&GameInstance::LoadingRoutine,this);//maybe I need some kind of staging method here
    auto UnloadingThread = std::thread(&GameInstance::UnloadingRoutine,this);


    //initialize imGuiFrame
    while(!OsmiumGL::ShouldClose()) {
    //TODO redo this
        OsmiumGL::RenderFrame();


        RenderDataUpdate();


    }

    AssetManager::Shutdown();
    SimulationThread.join();
    //ImGuiThread.join();->this need to be replaced somehow
    LoadingThread.join();
    UnloadingThread.join();
    AssetManager::UnloadAll(true);
    OsmiumGL::Shutdown();
    delete GameObjects;
}


void GameInstance::SetMainCamera(GameObjectHandle editor_camera) {//might do an override that takes a GOC_Camera pointer directly
    const auto& cameraObj =  GameObjects->get(editor_camera);
    const auto camComponent = cameraObj.GetComponent<GOC_Camera>();
    mainCamera = camComponent;

}
void GameInstance::SetMainCamera(GOC_Camera* camComp) {
    mainCamera = camComp;
}

void GameInstance::SetDirectionalLight(GOC_DirectionalLight *DirLightComp) {
    directionLight = DirLightComp;
}

glm::mat4 GameInstance::getMainCameraViewMatrix() {
    return instance->mainCamera->GetViewMatrix();
}


/*
 * Object will only be available next simulation tick, but a callback can be provided to recieve the
 **/
void GameInstance::CreateNewGameObject(GameObjectCreateInfo& createStruct, const std::function<void(GameObject*)>& callback) {//The function pointer is potentially dangerous if used from game objects

    std::unique_lock lock{creationQueueMutex};//should be enough for this
    gameObjectsCreationQueue.emplace(createStruct,callback);
}

void GameInstance::DestroyGameObject(GameObject *gameObject) {
    std::unique_lock lock {destructionQueueMutex};
    gameObjectsDestructionQueue.emplace(gameObject->Handle);
}

ResourceArray<GameObject, 2000> & GameInstance::GetGameObjects() const {
    return *GameObjects;
}



void GameInstance::LoadingRoutine() {
    AssetManager::LoadingRoutine();//should be ran privately by the asset manager directly
}

void GameInstance::UnloadingRoutine() {
    AssetManager::UnloadingRoutine();
}
