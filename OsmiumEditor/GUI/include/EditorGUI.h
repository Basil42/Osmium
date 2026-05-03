//
// Created by Shadow on 2/24/2025.
//

#ifndef EDITORGUI_H
#define EDITORGUI_H
#include <condition_variable>
#include <imgui.h>
#include <glm/vec3.hpp>

#include "SyncUtils.h"
#include "Base/config.h"
#include "Base/GameObject.h"


class GOC_Camera;
class InspectorWindow;
class GameObject;
class HierarchyWindow;
struct ImGuiSyncStruct;
class GameInstance;
struct ImGuiIO;

class EditorGUI {

    HierarchyWindow* hierarchyWindow = nullptr;
    InspectorWindow* inspectorWindow = nullptr;
    GameObjectHandle selectedGameObject = MAX_GAMEOBJECTS +1;
    float CameraSpeedDelta = 0.5f;
    glm::vec3 cameraSpeed = glm::vec3(0.0f);
    float CameraMaxSpeed = 10.0f;
    float MinCameraSpeed = 0.4f;
    float RotationSensitivity = 1.0f;

public:
    void Run();

    void CameraControls(ImGuiIO& io);

    void RenderImGuiFrameTask(std::mutex &ImguiMutex, const bool &ImGuiShouldShutoff,
                              std::condition_variable &ImguiNewFrameConditionVariable, bool &isImguiNewFrameReady);
    Sync::DependencySignal* CreateDependencySignal();
    void DeleteDependencySignal(Sync::DependencySignal* Signal);//should not require use
    void AddProviderSignal(Sync::DependencySignal* Signal);


    ImVec4 ImgGuiClearColor;
    bool showDemoWindow = true;
    bool showAnotherWindow = false;
    GameInstance* OsmiumInstance;
    GOC_Camera* EditorCamera;
    bool ShowHierarchy = true;
    bool ShowInspector = true;
    const ImGuiSyncStruct* SyncStruct;//TODO make the editor own the imgui sync struct

    std::vector<Sync::DependencySignal*> m_EditorProviders{};//shoudl only contain the simulation tick
    std::vector<Sync::DependencySignal> m_EditorConsumers;//also only the simulation tick


    EditorGUI() = delete;
    explicit EditorGUI(GameInstance * Instance);
};



#endif //EDITORGUI_H
