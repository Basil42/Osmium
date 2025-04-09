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
#include "GOComponents/GOC_PointLight.h"

GameInstance* GameInstance::instance = nullptr;
void GameInstance::GameLoop() {
    //double lastFrameTime = glfwGetTime();
    while (!OsmiumGL::ShouldClose()) {//might be thread unsafe to check this

        std::unique_lock<std::mutex> ImGuiLock(ImguiUpdateSync.mutex);
        ImguiUpdateSync.cv.wait(ImGuiLock, [this] { return ImguiUpdateSync.boolean == false; });


        SimulationSync.boolean = false;//bool was previously "sim is over"
        std::unique_lock<std::mutex> SimulationLock(SimulationSync.mutex);
        AssetManager::ProcessCallbacks();
        //game object creation
        while (!gameObjectsCreationQueue.empty()) {
            auto entry = gameObjectsCreationQueue.front();
            gameObjectsCreationQueue.pop();
            GameObject* newObj;
            const auto handle = GameObjects->emplace_new(newObj);
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
        isRenderUpdateOver = false;

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

void GameInstance::run(const std::string &appName) {

    instance = this;
    GameObjects = new ResourceArray<GameObject,MAX_GAMEOBJECTS>();
    OsmiumGL::Init(appName);
    //load the initial assets, probably in its own thread
    AssetManager::LoadAssetDatabase();
    //LoadInitialScene()
    //editor camera
    GameObjectCreateInfo CameraInfo;
    CameraInfo.name = "Camera";
    CameraInfo.parent = 0;
    auto SimulationThread = std::thread(&GameInstance::GameLoop,this);
    // auto ImGuiThread = std::thread(RenderImGuiFrameTask,this);
    auto LoadingThread = std::thread(&GameInstance::LoadingRoutine,this);//maybe I need some kind of staging method here
    auto UnloadingThread = std::thread(&GameInstance::UnloadingRoutine,this);

    //std::unique_lock<std::mutex> ImGuiLock(ImguiMutex,std::defer_lock);
    std::unique_lock<std::mutex> RenderDataLock(renderDataMutex, std::defer_lock);
    //initialize imGuiFrame
    while(!OsmiumGL::ShouldClose()) {

        OsmiumGL::RenderFrame(ImguiUpdateSync);
        //ImGuiLock.lock();
        //OsmiumGL::StartFrame();//poll glfw events and start imGui frame
        //imgui update is free to start if simulation is done
        //isImguiNewFrameReady = true;
        //ImGuiLock.unlock();
        //ImguiNewFrameConditionVariable.notify_one();
        //the function will wait until as late as possible to hold execution for imGui
        //OsmiumGL::EndFrame(ImguiMutex,ImguiUpdateConditionVariable,isImguiUpdateOver);
        RenderDataLock.lock();
        SimulationConditionVariable.wait(RenderDataLock,[this]() {return isSimOver;});

        RenderDataUpdate();
        isRenderUpdateOver = true;
        RenderDataLock.unlock();
        renderDataUpdateConditionVariable.notify_one();//probably notify all

    }
    //TODO flush imgui if needed
    //OsmiumGL::StartFrame();
    ImguiUpdateSync.boolean = true;// isImguiNewFrameReady = true;
    ImguiUpdateSync.cv.notify_one();// ImguiNewFrameConditionVariable.notify_one();
    //message the sim and imgui to shut off, these are technically nor thread safe but it might not matter here
    simShouldShutoff = true;
    ImGuiShouldShutoff = true;
    AssetManager::Shutdown();
    SimulationThread.join();
    //ImGuiThread.join();
    LoadingThread.join();
    UnloadingThread.join();
    AssetManager::UnloadAll(true);
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
    syncData.imGuiMutex = &ImguiUpdateSync.mutex;
    syncData.imGuiNewFrameConditionVariable = &ImguiUpdateSync.cv;// ImguiNewFrameConditionVariable;
    syncData.isImguiNewFrameReady = &ImguiUpdateSync.boolean;
    syncData.ImGuiShouldShutoff = &ImGuiShouldShutoff;
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
    AssetManager::LoadingRoutine();//should be ran privately by the asset manager directly
}

void GameInstance::UnloadingRoutine() {
    AssetManager::UnloadingRoutine();
}
