#include "Window.h"
#include <stdexcept>

void Window::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void APIENTRY Window::glDebugOutput(GLenum source, GLenum type, unsigned int id,
                                    GLenum severity, GLsizei length,
                                    const char *message, const void *userParam)
{
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    cout << "OpenGL Debug (" << id << "): " << message << endl;

    if (severity == GL_DEBUG_SEVERITY_HIGH || type == GL_DEBUG_TYPE_ERROR)
    {
        cout << "Severity: HIGH" << endl;
    }
}

Window::Window(const unsigned int width, const unsigned int height) : WIN_WIDTH(width), WIN_HEIGTH(height)
{
    if (!glfwInit())
    {
        cout << "Failed to initialize GLFW" << endl;
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef DEBUG
    std::cout << "Debug mode for openGL is on!\n";
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGTH, "Black Hole Simulation", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to open GLFW window" << endl;
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD" << endl;
        throw std::runtime_error("Failed to initialize GLAD");
    }

#ifdef DEBUG
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        cout << "Debug output enabled!" << endl;
    }
#endif

    glViewport(0, 0, WIN_WIDTH, WIN_HEIGTH);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

Window::~Window()
{
    glfwTerminate();
}