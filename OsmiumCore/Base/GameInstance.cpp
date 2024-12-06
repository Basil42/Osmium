//
// Created by nicolas.gerard on 2024-12-04.
//

#include "GameInstance.h"

#include <condition_variable>
#include <imgui.h>
#include <thread>

#include "OsmiumGL_API.h"


void GameInstance::GameLoop() {
    while (!OsmiumGL::ShouldClose()) {//might be thread unsafe to check this

        std::unique_lock<std::mutex> ImGuiLock(ImguiMutex);
        ImguiUpdateConditionVariable.wait(ImGuiLock, [this] { return isImguiUpdateOver; });

        isSimOver = false;
        std::unique_lock<std::mutex> SimulationLock(SimulationCompletionMutex);


        //Do simulation things
        isSimOver = true;
        SimulationLock.unlock();
        SimulationConditionVariable.notify_one();

        std::unique_lock<std::mutex> renderDataUpdateLock(renderDataMutex);
        renderDataUpdateConditionVariable.wait(renderDataUpdateLock, [this]() {return isRenderUpdateOver;});


    }
}

void GameInstance::run() {

    OsmiumGL::Init;
    //load the initial assets, probably in its own thread
    //LoadInitialScene()



    auto SimulationThread = std::thread(GameLoop,this);

    std::unique_lock<std::mutex> ImGuiLock(ImguiMutex,std::defer_lock);
    std::unique_lock<std::mutex> RenderDataLock(renderDataMutex, std::defer_lock);
    auto ImGuiThread = std::thread(RenderImGuiFrameTask,this);
    while(!OsmiumGL::ShouldClose()) {
        ImGuiLock.lock();
        OsmiumGL::StartFrame();//poll glfw events and start imGui frame
        //imgui update is free to start if simulation is done
        isImguiNewFrameReady = true;
        ImGuiLock.unlock();
        ImguiUpdateConditionVariable.notify_one();
        //the function will wait until as late as possible to hold execution for imGui
        OsmiumGL::EndFrame(ImguiMutex,ImguiUpdateConditionVariable,isImguiUpdateOver);
        RenderDataLock.lock();
        SimulationConditionVariable.wait(RenderDataLock,[this]() {return isSimOver;});

        RenderDateUpdate();
        isRenderUpdateOver = true;
        renderDataUpdateConditionVariable.notify_one();
        RenderDataLock.unlock();

    }
    // io = ImGui::GetIO();
    // while (!OsmiumGL::ShouldClose()) {
    //     OsmiumGL::StartFrame();
    //     RenderImGuiFrameTask();//ideally I would do that wherever and send it as a message to the render thread
    //     OsmiumGL::EndFrame();
    // }
    // OsmiumGL::Shutdown();
}

void GameInstance::RenderImGuiFrameTask() {


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
    ImGui::Render();
}
