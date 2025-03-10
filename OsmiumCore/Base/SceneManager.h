//
// Created by Shadow on 12/5/2024.
//

#ifndef SCENE_H
#define SCENE_H
#include <string>
#include <vector>


typedef unsigned long SceneId;
class SceneManager {
    std::vector<SceneId> Scenes;
    std::vector<SceneId> LoadingScenes;
    std::vector<SceneId> UnloadingScene;
public:
    SceneId LoadScene(std::string);
    bool isSceneLoaded();
    bool isSceneLoading();
};



#endif //SCENE_H
