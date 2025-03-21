//
// Created by Shadow on 2/24/2025.
//

#include "EditorGUI.h"

#include <condition_variable>
#include <imgui.h>
#include <mutex>

//most of these include will be moved to more specialized inspector and window classes
#include "HierarchyWindow.h"
#include "InspectorWindow.h"
#include "OsmiumGL_API.h"
#include "AssetManagement/Asset.h"
#include "Base/GameInstance.h"
#include "Base/GameObject.h"
#include "Base/GameObjectCreation.h"
#include "GOComponents/GOC_MeshRenderer.h"
#include "GOComponents/GOC_Transform.h"
#include "GOComponents/GOC_Camera.h"

void EditorGUI::Run() {

    hierarchyWindow = new HierarchyWindow(OsmiumInstance,selectedGameObject);
    inspectorWindow = new InspectorWindow(OsmiumInstance, selectedGameObject);

    RenderImGuiFrameTask(*SyncStruct->imGuiMutex,*SyncStruct->ImGuiShouldShutoff,*SyncStruct->imGuiNewFrameConditionVariable,
        *SyncStruct->isImguiNewFrameReady,*SyncStruct->isImguiUpdateOver,*SyncStruct->ImguiUpdateConditionVariable);

    delete inspectorWindow;
    delete hierarchyWindow;

}

void EditorGUI::CameraControls(ImGuiIO &io) {
    if (ImGui::IsWindowFocused())return;//ignore input
    glm::vec2 PositionInput = glm::vec2(0.0f);
    float YawInput = 0.0f;
    float PitchInput = 0.0f;
    if (ImGui::IsKeyDown(ImGuiKey_W)) PositionInput.y += 1.0f;
    if (ImGui::IsKeyDown(ImGuiKey_S)) PositionInput.y -= 1.0f;
    if (ImGui::IsKeyDown(ImGuiKey_D)) PositionInput.x += 1.0f;
    if (ImGui::IsKeyDown(ImGuiKey_A)) PositionInput.x -= 1.0f;
    if (PositionInput == glm::vec2(0.0f)) {;
        cameraSpeed = cameraSpeed * glm::pow(0.1f,io.DeltaTime);
        if (cameraSpeed.length() < MinCameraSpeed)cameraSpeed = glm::vec3(0.0f);
    }

    cameraSpeed = glm::clamp(cameraSpeed + glm::vec3(PositionInput.x,0.0f,PositionInput.y) * CameraSpeedDelta * io.DeltaTime,-CameraMaxSpeed*io.DeltaTime,CameraMaxSpeed * io.DeltaTime);
    auto rotInput = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right,0.0f);
    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
    YawInput = rotInput.x;
    PitchInput = rotInput.y;

    auto transformValue = EditorCamera->GetTransform();
    transformValue = glm::translate(transformValue,cameraSpeed);
    transformValue = glm::rotate(transformValue,YawInput * RotationSensitivity *io.DeltaTime,glm::vec3(0.0f,1.0f,0.0f));
    transformValue = glm::rotate(transformValue,PitchInput * RotationSensitivity * io.DeltaTime,glm::vec3(1.0f,0.0f,0.0f));
    EditorCamera->SetTransform(transformValue);
    ImGui::Begin("Camera Controls");
    ImGui::InputFloat2("Rot input", &rotInput[0]);
    ImGui::End();

}

void EditorGUI::RenderImGuiFrameTask(std::mutex &ImguiMutex, const bool &ImGuiShouldShutoff,
                                     std::condition_variable &ImguiNewFrameConditionVariable,
                                     bool &isImguiNewFrameReady, bool
                                     &isImguiUpdateOver, std::condition_variable &ImguiUpdateConditionVariable) {


    std::unique_lock<std::mutex> startFrameLock{ ImguiMutex, std::defer_lock };

    //should request an editor camera here
    GameObjectCreateInfo editorCameraInfo {
    .name = "EditorCamera",
    .parent = 0};
    OsmiumInstance->CreateNewGameObject(editorCameraInfo,[this](GameObject* go) {
        go->Addcomponent<GOC_Transform>();
        EditorCamera = go->Addcomponent<GOC_Camera>();

        OsmiumInstance->SetMainCamera(EditorCamera);
    });

    while (!ImGuiShouldShutoff) {
        startFrameLock.lock();
        ImguiNewFrameConditionVariable.wait(startFrameLock,[this, &isImguiNewFrameReady]() {return isImguiNewFrameReady;});
        isImguiNewFrameReady = false;

        ImGuiIO io = ImGui::GetIO();

        CameraControls(io);

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
        //TODO clean this up and send the different functionnality to appropriate widgets
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
            ImGui::ColorEdit3("clear color", (float*)(&ImgGuiClearColor)); // Edit 3 floats representing a color

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

                GameObjectCreateInfo defaultObjectInfo;
                defaultObjectInfo.name = "default";
                defaultObjectInfo.parent = 0;//I should probably pre allocate a 0 handle game object just to be safe
                OsmiumInstance->CreateNewGameObject(defaultObjectInfo,[](GameObject* gameObject) {
                    gameObject->Addcomponent<GOC_Transform>();
                    gameObject->Addcomponent<GOC_MeshRenderer>([](GOC_MeshRenderer* renderer) {
                        renderer->SetMaterial(OsmiumGL::GetBlinnPhongHandle(),true);
                        renderer->SetMeshAsset(Asset::getAssetId("../OsmiumGL/DefaultResources/models/monkey.obj"));
                    });
                });

                    // GameObject* defaultObject = CreateNewGameObject();
                    // defaultObject->Addcomponent<GOC_Transform>();
                    // const auto defaultGOMeshRenderer = defaultObject->Addcomponent<GOC_MeshRenderer>();
                    // defaultGOMeshRenderer->SetMaterial(OsmiumGL::GetBlinnPhongHandle(), true);
                    // defaultGOMeshRenderer->SetMaterialInstance(OsmiumGL::GetBlinnPhongDefaultInstance());//assuming this will be the handle for the default Blinn Phong Mat
                    // defaultGOMeshRenderer->SetMeshAsset(Asset::getAssetId("../OsmiumGL/DefaultResources/models/monkey.obj"));//should finish the setup on its own;

            }
            ImGui::Checkbox("Entity hierarchy", &ShowHierarchy);
            ImGui::Checkbox("Inspector", &ShowInspector);
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
        if (ShowHierarchy) {
            hierarchyWindow->Render(io);
        }
        if (ShowInspector)inspectorWindow->Render(io);
        OsmiumGL::ImguiEndImGuiFrame();
        //sync
        isImguiUpdateOver = true;
        startFrameLock.unlock();
        ImguiUpdateConditionVariable.notify_all();
    }
}

EditorGUI::EditorGUI(const ImGuiSyncStruct &im_gui_sync_struct,GameInstance* Instance) {
    SyncStruct = &im_gui_sync_struct;
    OsmiumInstance = Instance;
}
