//
// Created by Shadow on 2/24/2025.
//

#ifndef EDITORGUI_H
#define EDITORGUI_H
#include <condition_variable>


struct ImGuiSyncStruct;
class GameInstance;
struct ImGuiIO;
struct ImVec4;

class EditorGUI {

  public:
    void Run();

    void RenderImGuiFrameTask(std::mutex &ImguiMutex, const bool &ImGuiShouldShutoff,
                              std::condition_variable &ImguiNewFrameConditionVariable, bool &isImguiNewFrameReady, bool &isImguiUpdateOver, std::
                              condition_variable
                              &ImguiUpdateConditionVariable);


    ImVec4* ImgGuiClearColor;
    bool showDemoWindow;
    bool showAnotherWindow;
    GameInstance* OsmiumInstance;
    bool ShowHierarchy;
    ImGuiSyncStruct* SyncStruct;
};



#endif //EDITORGUI_H
