#pragma once

#include <condition_variable>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../World.hpp"

template <class T>
class CallbackHandler {

};

class Display {
    World *world;
    GLFWwindow *window;
    static Display *instance;
    //glfw callbacks
    static void callback_key(GLFWwindow *window, int key, int scancode, int action, int mode);
public:
    Display(int w, int h, std::string title, World *worldPtr);

    ~Display();

    void MainLoop();

    bool IsClosed();

    void SetPlayerPos(float x, float y);
};

