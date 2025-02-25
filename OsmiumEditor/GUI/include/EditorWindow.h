//
// Created by nicolas.gerard on 2025-02-25.
//

#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H


struct ImGuiIO;

class EditorWindow {
protected:
    ~EditorWindow() = default;

private:
    public:
    virtual void Render(ImGuiIO& io) = 0;
};



#endif //EDITORWINDOW_H
