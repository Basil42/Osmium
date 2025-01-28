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


void GameInstance::GameLoop() {
    //double lastFrameTime = glfwGetTime();
    while (!OsmiumGL::ShouldClose()) {//might be thread unsafe to check this

        std::unique_lock<std::mutex> ImGuiLock(ImguiMutex);
        ImguiUpdateConditionVariable.wait(ImGuiLock, [this] { return isImguiUpdateOver; });

        isSimOver = false;
        std::unique_lock<std::mutex> SimulationLock(SimulationCompletionMutex);
        AssetManager::ProcessCallbacks();

        //Do simulation things
        for (auto& game_object : gameObjects) {
            game_object.UpdateComponents();
        }

        isSimOver = true;
        SimulationLock.unlock();
        ImGuiLock.unlock();
        SimulationConditionVariable.notify_one();

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

    OsmiumGL::Init();
    //load the initial assets, probably in its own thread
    AssetManager::LoadAssetDatabase();
    //LoadInitialScene()
    //editor camera
    auto CameraGO = CreateNewGameObject();
    auto mainCamTransform = CameraGO->Addcomponent<GOC_Transform>();
    mainCamera = CameraGO->Addcomponent<GOC_Camera>();
    //mainCamTransform->SetTransformMatrix(glm::lookAt(glm::vec3(2.0f,2.0f,2.0f),glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f)));


    auto SimulationThread = std::thread(GameLoop,this);
    auto ImGuiThread = std::thread(RenderImGuiFrameTask,this);
    auto LoadingThread = std::thread(LoadingRoutine,this);//maybe I need some kind of staging method here

    std::unique_lock<std::mutex> ImGuiLock(ImguiMutex,std::defer_lock);
    std::unique_lock<std::mutex> RenderDataLock(renderDataMutex, std::defer_lock);
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
        renderDataUpdateConditionVariable.notify_one();

    }

    OsmiumGL::StartFrame();//misnamed method, this is called to flush the Imgui thread and force it to exit
    isImguiNewFrameReady = true;
    ImguiNewFrameConditionVariable.notify_one();
    //message the sim and imgui to shut off, these are technically nor thread safe but it might not matter here
    simShouldShutoff = true;
    ImGuiShouldShutoff = true;
    AssetManager::Shutdown();
    SimulationThread.join();
    ImGuiThread.join();
    LoadingThread.join();
    AssetManager::UnloadAll();
    OsmiumGL::Shutdown();
    // io = ImGui::GetIO();
    // while (!OsmiumGL::ShouldClose()) {
    //     OsmiumGL::StartFrame();
    //     RenderImGuiFrameTask();//ideally I would do that wherever and send it as a message to the render thread
    //     OsmiumGL::EndFrame();
    // }
    // OsmiumGL::Shutdown();
}

GameObject * GameInstance::CreateNewGameObject() {
    gameObjects.emplace_back();
    return &gameObjects.back();
}

void GameInstance::RenderImGuiFrameTask() {


    std::unique_lock<std::mutex> startFrameLock{ ImguiMutex, std::defer_lock };

    while (!ImGuiShouldShutoff) {
        startFrameLock.lock();
        ImguiNewFrameConditionVariable.wait(startFrameLock,[this]() {return isImguiNewFrameReady;});
        isImguiNewFrameReady = false;

        io = ImGui::GetIO();
        static bool warningTriggered = false;
        if(warningTriggered) {
            static float lastAbnormalDeltaTime = io.DeltaTime;
            ImGui::Begin("frame drop alert");
            if(io.DeltaTime > 1.0f/60.0f)lastAbnormalDeltaTime = io.DeltaTime;
            ImGui::Text("deltaTime: %f", lastAbnormalDeltaTime);
            ImGui::End();
        }else {
            warningTriggered = io.DeltaTime > 1.0f/60.0f;
        }
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (showDemoWindow)
            ImGui::ShowDemoWindow(&showDemoWindow);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &showDemoWindow);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &showAnotherWindow);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&(ImgGuiClearColor)); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);//this shoudl probably be a reference to stay up to date
            static unsigned int frameNumber = 0;
            ImGui::Text("Frame Number: %u", frameNumber++);
            if(ImGui::Button("Create Default Entity")) {
                //set the transform to something convenient here
                //this should happen on the sim thread

                    GameObject* defaultObject = CreateNewGameObject();
                    defaultObject->Addcomponent<GOC_Transform>();
                    const auto defaultGOMeshRenderer = defaultObject->Addcomponent<GOC_MeshRenderer>();
                    defaultGOMeshRenderer->SetMaterial(OsmiumGL::GetBlinnPhongHandle(), false);
                    defaultGOMeshRenderer->SetMaterialInstance(OsmiumGL::GetBlinnPhongDefaultInstance());//assuming this will be the handle for the default Blinn Phong Mat
                    defaultGOMeshRenderer->SetMeshAsset(Asset::getAssetId("../OsmiumGL/DefaultResources/models/monkey.obj"));//should finish the setup on its own;

            }
            ImGui::End();

        }

        // 3. Show another simple window.
        if (showAnotherWindow)
        {
            ImGui::Begin("Another Window", &showAnotherWindow);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                showAnotherWindow = false;
            ImGui::End();
        }
        OsmiumGL::ImguiEndImGuiFrame();
        //sync
        isImguiUpdateOver = true;
        startFrameLock.unlock();
        ImguiUpdateConditionVariable.notify_all();
    }
}

void GameInstance::LoadingRoutine() {
    AssetManager::LoadingRoutine();
}
