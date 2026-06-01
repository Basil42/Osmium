//
// Created by Shadow on 2/24/2025.
//

#include "EditorGUI.h"

#include <condition_variable>
#include <imgui.h>

//most of these include will be moved to more specialized inspector and window classes
#include "HierarchyWindow.h"
#include "imgui_internal.h"
#include "InspectorWindow.h"
#include "OsmiumGL_API.h"
#include "AssetManagement/Asset.h"
#include "Base/GameInstance.h"
#include "GOComponents/GOC_MeshRenderer.h"
#include "GOComponents/GOC_Transform.h"
#include "GOComponents/GOC_Camera.h"
#include "GOComponents/GOC_DirectionalLight.h"

void EditorGUI::Run() {
    m_hierarchyWindow = new HierarchyWindow(OsmiumInstance, selectedGameObject);
    m_inspectorWindow = new InspectorWindow(OsmiumInstance, selectedGameObject);

    GameObjectCreateInfo editorCameraInfo{
        .name = "Editor Camera",
        .parent = 0
    };
    OsmiumInstance->CreateNewGameObject(editorCameraInfo, [this](GameObject *go) {
        go->Addcomponent<GOC_Transform>();
        EditorCamera = go->Addcomponent<GOC_Camera>();
        go->hiddenInEditor = true;
        OsmiumInstance->SetMainCamera(EditorCamera);
    });
    OsmiumGL::UpdateMainCameraSettings(45.0f);//initializing the fov, should be better managed through the camera class
    std::thread GUIThread = std::thread(&EditorGUI::RenderImGuiFrameTask, this);
    OsmiumInstance->run();

    GUIThread.join();
    delete m_inspectorWindow;
    delete m_hierarchyWindow;
}

void EditorGUI::CameraControls(ImGuiIO &io) {
    if (ImGui::IsWindowFocused())return; //ignore input
    glm::vec2 PositionInput = glm::vec2(0.0f);
    float YawInput = 0.0f;
    float PitchInput = 0.0f;
    if (ImGui::IsKeyDown(ImGuiKey_W)) PositionInput.y += 1.0f;
    if (ImGui::IsKeyDown(ImGuiKey_S)) PositionInput.y -= 1.0f;
    if (ImGui::IsKeyDown(ImGuiKey_D)) PositionInput.x += 1.0f;
    if (ImGui::IsKeyDown(ImGuiKey_A)) PositionInput.x -= 1.0f;
    if (PositionInput == glm::vec2(0.0f)) {
        ;
        cameraSpeed = cameraSpeed * glm::pow(0.001f, io.DeltaTime);
        if (cameraSpeed.length() < MinCameraSpeed)cameraSpeed = glm::vec3(0.0f);
    }
    auto transformComp = EditorCamera->GetTransformComponent();
    //there should be faster way to compute these
    auto rot = transformComp->getRotation();
    auto forwardVector = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) * rot;
    auto upVector = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) * rot;
    auto rightVector = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) * rot;
    cameraSpeed = glm::clamp(
        cameraSpeed + glm::vec3(PositionInput.x, 0.0f, -PositionInput.y) * CameraSpeedDelta * io.DeltaTime,
        -CameraMaxSpeed * io.DeltaTime, CameraMaxSpeed * io.DeltaTime);
    auto rotInput = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right, 0.0f);
    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
    YawInput = -rotInput.x;
    PitchInput = -rotInput.y;

    auto transformMatrix = transformComp->getTransformMatrix();
    transformMatrix = glm::translate(transformMatrix, cameraSpeed);
    transformMatrix = glm::rotate(transformMatrix, YawInput * RotationSensitivity * io.DeltaTime,
                                  glm::vec3(0.0f, 1.0f, 0.0f));
    transformMatrix = glm::rotate(transformMatrix, PitchInput * RotationSensitivity * io.DeltaTime,
                                  glm::vec3(1.0f, 0.0f, 0.0f));
    EditorCamera->SetTransform(transformMatrix);
    ImGui::Begin("Camera Controls");
    ImGui::InputFloat2("Rot input", &rotInput[0]);
    ImGui::End();
}

void EditorGUI::RenderImGuiFrameTask() {
    //There should not be a need to actually display that frame
    while (!OsmiumGL::ShouldClose()) {//condition check can lock
        Sync::SynchronizationManager::Wait(Sync::SYNC_STAGE_RENDER_IMGUI_FRAME_START,m_EditorFrameCounter);//will pass through on the first frame
        ImGuiID dockID = ImGui::DockSpaceOverViewport();
        //conditionally create docking layout, might need to be moved to init
        if (!ImGui::DockBuilderGetNode(dockID)->IsSplitNode() && !ImGui::FindWindowByName("Viewport")) {
            ImGui::DockBuilderGetCentralNode(dockID)->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;// Remove "Tab" from the central node
            ImGuiID leftID = ImGui::DockBuilderSplitNode(dockID, ImGuiDir_Left, 0.2f, nullptr, &dockID);
            ImGui::DockBuilderDockWindow("Viewport", dockID); // Dock "Viewport" to  central node
            ImGuiID rightId = ImGui::DockBuilderSplitNode(dockID, ImGuiDir_Right, 0.2f, nullptr, &dockID);
            ImGui::DockBuilderDockWindow("Inspector", rightId);
            // Split the central node
            ImGui::DockBuilderDockWindow("Hierarchy", leftID); // Dock "Hierarchy" to the left node
        }
        // We define "viewport" with no padding and retrieve the rendering area
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImGui::End();
        ImGui::PopStyleVar();
        // [optional] Show the menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("vSync", "", &OsmiumGL::GetVsync()))
                    OsmiumGL::RequestSwapchainRebuild(); // Recreate the swapchain with the new vSync setting
                ImGui::Separator();
                if (ImGui::MenuItem("Exit"))
                    OsmiumGL::CloseWindow();
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (ImGui::Begin("Viewport")) {
            auto size = ImGui::GetContentRegionAvail();
            ImGui::Image(OsmiumGL::GetImGuiRenderTarget(size), size);//image index can be changed here to render one of the framebuffer in the viewport

            //overlay stuff, might remove later
            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        }
        ImGui::End();
        //Proted for the sample as an example
        m_hierarchyWindow->Render(ImGui::GetIO());
        m_inspectorWindow->Render(ImGui::GetIO());

        OsmiumGL::ImguiEndImGuiFrame();
        m_EditorFrameCounter = Sync::SynchronizationManager::Signal(Sync::SYNC_STAGE_EDITOR);
    }
}

EditorGUI::EditorGUI() {
    OsmiumInstance = new GameInstance();
    //TODO create editor camera here
}



EditorGUI::~EditorGUI() {
    delete OsmiumInstance;
}
