//
// Created by Shadow on 2/24/2025.
//

#ifndef EDITORGUI_H
#define EDITORGUI_H
#include <array>
#include <condition_variable>
#include <imgui.h>
#include <span>
#include <glm/vec3.hpp>

#include "SyncUtils.h"
#include "../../../OsmiumCommon/include/CommonConfig.h"
#include "Base/GameObject.h"


class GOC_Camera;
class InspectorWindow;
class GameObject;
class HierarchyWindow;
struct ImGuiSyncStruct;
class GameInstance;
struct ImGuiIO;

class EditorGUI {

    HierarchyWindow* m_hierarchyWindow = nullptr;
    InspectorWindow* m_inspectorWindow = nullptr;
    GameObjectHandle selectedGameObject = MAX_GAMEOBJECTS +1;
    float CameraSpeedDelta = 0.5f;
    glm::vec3 cameraSpeed = glm::vec3(0.0f);
    float CameraMaxSpeed = 10.0f;
    float MinCameraSpeed = 0.4f;
    float RotationSensitivity = 1.0f;
    uint_fast64_t m_EditorFrameCounter=0;

public:
    void Run();

    void CameraControls(ImGuiIO& io);

    void RenderImGuiFrameTask();


    ImVec4 ImgGuiClearColor;
    bool showDemoWindow = true;
    bool showAnotherWindow = false;
    GameInstance* OsmiumInstance;
    GOC_Camera* EditorCamera;
    bool ShowHierarchy = true;
    bool ShowInspector = true;

    EditorGUI();
    ~EditorGUI();
};



#endif //EDITORGUI_H
