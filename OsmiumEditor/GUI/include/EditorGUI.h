//
// Created by Shadow on 2/24/2025.
//

#ifndef EDITORGUI_H
#define EDITORGUI_H
#include <condition_variable>
#include <imgui.h>

#include "Base/config.h"
#include "Base/GameObject.h"


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

public:
    void Run();

    void RenderImGuiFrameTask(std::mutex &ImguiMutex, const bool &ImGuiShouldShutoff,
                              std::condition_variable &ImguiNewFrameConditionVariable, bool &isImguiNewFrameReady, bool &isImguiUpdateOver, std::
                              condition_variable
                              &ImguiUpdateConditionVariable);


    ImVec4 ImgGuiClearColor;
    bool showDemoWindow = true;
    bool showAnotherWindow = false;
    GameInstance* OsmiumInstance;
    bool ShowHierarchy = true;
    bool ShowInspector = true;
    const ImGuiSyncStruct* SyncStruct;


    EditorGUI() = delete;
    explicit EditorGUI(const ImGuiSyncStruct & im_gui_sync_struct,GameInstance * Instance);
};



#endif //EDITORGUI_H
