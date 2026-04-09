#pragma once

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window
{
private:
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id,
                                       GLenum severity, GLsizei length,
                                       const char *message, const void *userParam);

    GLFWwindow *window;
    const unsigned int width;
    const unsigned int height;
    bool glfwInitialized;
    std::string lastError;

    bool initializeGlfw();
    bool createContext();
    bool initializeGlad();
    bool validateRuntimeCapabilities();
    void shutdown();

public:
    Window(unsigned int width, unsigned int height);
    ~Window();

    bool initialize();
    GLFWwindow *p_GLFWwindow() const { return window; }
    unsigned int getWidth() const { return width; }
    unsigned int getHeight() const { return height; }
    const std::string &getLastError() const { return lastError; }
};
