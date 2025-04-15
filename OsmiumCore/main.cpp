
#include "Base/GameInstance.h"
//
// Created by Shadow on 11/28/2024.
//
int main(int argc, char *argv[]) {
    GameInstance game;
    //TODO add an option to let the engine manage the ImgGUI frame content itself (debug GUI mode)
    game.run("Core test");
    system("pause");
}
