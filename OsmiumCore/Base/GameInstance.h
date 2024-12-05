//
// Created by nicolas.gerard on 2024-12-04.
//

#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H
#include <imgui.h>


class GameInstance {
    void RenderImGuiFrame();
    //temp
    bool showDemoWindow;
    bool showAnotherWindow;
    //Ecapsulate these two fields into a separate struct
    ImVec4 ImgGuiClearColor;
    ImGuiIO io;

public:
    

    void run();

};

struct gameConfig {

};

#endif //GAMEINSTANCE_H
