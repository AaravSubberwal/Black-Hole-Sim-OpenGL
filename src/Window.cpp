#include "Window.h"

#include <iostream>

void Window::framebuffer_size_callback(GLFWwindow *, int width, int height)
{
    glViewport(0, 0, width, height);
}

void APIENTRY Window::glDebugOutput(GLenum, GLenum type, unsigned int id,
                                    GLenum severity, GLsizei,
                                    const char *message, const void *)
{
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
    {
        return;
    }

    std::cout << "OpenGL Debug (" << id << "): " << message << std::endl;
    if (severity == GL_DEBUG_SEVERITY_HIGH || type == GL_DEBUG_TYPE_ERROR)
    {
        std::cout << "Severity: HIGH" << std::endl;
    }
}

Window::Window(unsigned int width, unsigned int height)
    : window(nullptr), width(width), height(height), glfwInitialized(false)
{
}

Window::~Window()
{
    shutdown();
}

bool Window::initialize()
{
    return initializeGlfw() && createContext() && initializeGlad() && validateRuntimeCapabilities();
}

bool Window::initializeGlfw()
{
    if (!glfwInit())
    {
        lastError = "Failed to initialize GLFW.";
        return false;
    }

    glfwInitialized = true;
    return true;
}

bool Window::createContext()
{
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), "Black Hole Simulation", nullptr, nullptr);
    if (window == nullptr)
    {
        lastError = "Failed to create the GLFW window.";
        shutdown();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    return true;
}

bool Window::initializeGlad()
{
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        lastError = "Failed to initialize GLAD.";
        shutdown();
        return false;
    }

#ifdef DEBUG
    int flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return true;
}

bool Window::validateRuntimeCapabilities()
{
    if (!GLAD_GL_VERSION_4_3)
    {
        const char *version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
        lastError = "OpenGL 4.3+ is required. Detected version: ";
        lastError += (version != nullptr) ? version : "unknown";
        shutdown();
        return false;
    }

    return true;
}

void Window::shutdown()
{
    if (window != nullptr)
    {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    if (glfwInitialized)
    {
        glfwTerminate();
        glfwInitialized = false;
    }
}
