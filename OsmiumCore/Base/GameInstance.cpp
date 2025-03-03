//
// Created by nicolas.gerard on 2024-12-04.
//

#include "GameInstance.h"

#include <condition_variable>
#include <imgui.h>
#include <thread>

#include "OsmiumGL_API.h"
#include "../AssetManagement/AssetManager.h"
#include "../AssetManagement/AssetType/MeshAsset.h"
#include "../GOComponents/GOC_MeshRenderer.h"
#include "../GOComponents/GOC_Transform.h"
#include "../GOComponents/GOC_Camera.h"
#include "ResourceArray.h"

GameInstance* GameInstance::instance = nullptr;
void GameInstance::GameLoop() {
    //double lastFrameTime = glfwGetTime();
    while (!OsmiumGL::ShouldClose()) {//might be thread unsafe to check this

        std::unique_lock<std::mutex> ImGuiLock(ImguiMutex);
        ImguiUpdateConditionVariable.wait(ImGuiLock, [this] { return isImguiUpdateOver; });

        isSimOver = false;
        std::unique_lock<std::mutex> SimulationLock(SimulationCompletionMutex);
        AssetManager::ProcessCallbacks();
        //game object creation
        while (!gameObjectsCreationQueue.empty()) {
            auto entry = gameObjectsCreationQueue.front();
            gameObjectsCreationQueue.pop();
            GameObject* newObj;
            auto handle = GameObjects->emplace_new(newObj);
            newObj->Name = entry.first.name;
            newObj->Handle = handle;
            entry.second(newObj);
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
        isSimOver = true;
        SimulationLock.unlock();
        ImGuiLock.unlock();
        SimulationConditionVariable.notify_one();//might need to be turned into a notify all

        std::unique_lock<std::mutex> renderDataUpdateLock(renderDataMutex);
        renderDataUpdateConditionVariable.wait(renderDataUpdateLock, [this]() {return isRenderUpdateOver;});


    }
}

void GameInstance::RenderDataUpdate() {
    if (mainCamera == nullptr) return;
    mainCamera->RenderUpdate();
    GOC_MeshRenderer::GORenderUpdate();
}

void GameInstance::run() {

    instance = this;
    GameObjects = new ResourceArray<GameObject,MAX_GAMEOBJECTS>();
    OsmiumGL::Init();
    //load the initial assets, probably in its own thread
    AssetManager::LoadAssetDatabase();
    //LoadInitialScene()
    //editor camera
    GameObjectCreateInfo CameraInfo;
    CameraInfo.name = "Camera";
    CameraInfo.parent = 0;
    //auto mainCamTransform = CameraGO->Addcomponent<GOC_Transform>();
    //mainCamera = CameraGO->Addcomponent<GOC_Camera>();
    CreateNewGameObject(CameraInfo, [this](GameObject* game_object) {
        game_object->Addcomponent<GOC_Transform>();
        auto camComponent = game_object->Addcomponent<GOC_Camera>();
        mainCamera = camComponent;
    });
    //mainCamTransform->SetTransformMatrix(glm::lookAt(glm::vec3(2.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f)));


    auto SimulationThread = std::thread(GameLoop,this);
    // auto ImGuiThread = std::thread(RenderImGuiFrameTask,this);
    auto LoadingThread = std::thread(LoadingRoutine,this);//maybe I need some kind of staging method here

    std::unique_lock<std::mutex> ImGuiLock(ImguiMutex,std::defer_lock);
    std::unique_lock<std::mutex> RenderDataLock(renderDataMutex, std::defer_lock);
    //initialize imGuiFrame
    while(!OsmiumGL::ShouldClose()) {
        ImGuiLock.lock();
        OsmiumGL::StartFrame();//poll glfw events and start imGui frame
        //imgui update is free to start if simulation is done
        isImguiNewFrameReady = true;
        ImGuiLock.unlock();
        ImguiNewFrameConditionVariable.notify_one();
        //the function will wait until as late as possible to hold execution for imGui
        OsmiumGL::EndFrame(ImguiMutex,ImguiUpdateConditionVariable,isImguiUpdateOver);
        RenderDataLock.lock();
        SimulationConditionVariable.wait(RenderDataLock,[this]() {return isSimOver;});

        RenderDataUpdate();
        isRenderUpdateOver = true;
        RenderDataLock.unlock();
        renderDataUpdateConditionVariable.notify_one();//probably notify all

    }

    OsmiumGL::StartFrame();//misnamed method, this is called to flush the Imgui thread and force it to exit
    isImguiNewFrameReady = true;
    ImguiNewFrameConditionVariable.notify_one();
    //message the sim and imgui to shut off, these are technically nor thread safe but it might not matter here
    simShouldShutoff = true;
    ImGuiShouldShutoff = true;
    AssetManager::Shutdown();
    SimulationThread.join();
    //ImGuiThread.join();
    LoadingThread.join();
    AssetManager::UnloadAll();
    OsmiumGL::Shutdown();
    delete GameObjects;
    // io = ImGui::GetIO();
    // while (!OsmiumGL::ShouldClose()) {
    //     OsmiumGL::StartFrame();
    //     RenderImGuiFrameTask();//ideally I would do that wherever and send it as a message to the render thread
    //     OsmiumGL::EndFrame();
    // }
    // OsmiumGL::Shutdown();
}

void GameInstance::getImGuiSyncInfo(ImGuiSyncStruct& syncData) {
    syncData.imGuiMutex = &ImguiMutex;
    syncData.imGuiNewFrameConditionVariable = &ImguiNewFrameConditionVariable;
    syncData.isImguiNewFrameReady = &isImguiNewFrameReady;
    syncData.isImguiUpdateOver = &isImguiUpdateOver;
    syncData.ImGuiShouldShutoff = &ImGuiShouldShutoff;
    syncData.ImguiUpdateConditionVariable = &ImguiUpdateConditionVariable;
}

glm::mat4 GameInstance::getMainCameraViewMatrix() {
    return instance->mainCamera->GetViewMatrix();
}


/*
 * Object will only be available next simulation tick, but a callback can be provided to recieve the
 **/
void GameInstance::CreateNewGameObject(GameObjectCreateInfo& createStruct, const std::function<void(GameObject*)>& callback) {//The function pointer is potentially dangerous if used from game objects

    std:std::unique_lock lock{creationQueueMutex};//should be enough for this
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
    AssetManager::LoadingRoutine();
}
