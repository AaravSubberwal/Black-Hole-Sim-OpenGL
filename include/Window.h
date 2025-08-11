#pragma once
#include <iostream>

#include <glad/glad.h>
#include <glfw/glfw3.h>

using namespace std;

class Window
{
private:
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, 
                                     GLenum severity, GLsizei length, 
                                     const char *message, const void *userParam);
    
    GLFWwindow *window;
    const unsigned int WIN_WIDTH;
    const unsigned int WIN_HEIGTH;

public:
    inline GLFWwindow *p_GLFWwindow() { return window; }
    inline unsigned int getWidth() { return WIN_WIDTH; }
    inline unsigned int getHeight() { return WIN_HEIGTH; }

    Window(const unsigned int width, const unsigned int height);
    ~Window();
};
