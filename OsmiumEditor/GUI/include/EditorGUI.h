//
// Created by Shadow on 2/24/2025.
//

#ifndef EDITORGUI_H
#define EDITORGUI_H
#include <condition_variable>
#include <imgui.h>
#include <glm/vec3.hpp>

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


    ImVec4 ImgGuiClearColor;
    bool showDemoWindow = true;
    bool showAnotherWindow = false;
    GameInstance* OsmiumInstance;
    GOC_Camera* EditorCamera;
    bool ShowHierarchy = true;
    bool ShowInspector = true;
    const ImGuiSyncStruct* SyncStruct;


    EditorGUI() = delete;
    explicit EditorGUI(const ImGuiSyncStruct & im_gui_sync_struct,GameInstance * Instance);
};



#endif //EDITORGUI_H
