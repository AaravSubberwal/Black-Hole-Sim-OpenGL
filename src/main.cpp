#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "AppPaths.h"
#include "BlackHole.h"
#include "Camera.h"
#include "Star.h"
#include "Window.h"
#include "shader.h"

namespace
{
constexpr int INITIAL_RENDER_WIDTH = 1280;
constexpr int INITIAL_RENDER_HEIGHT = 720;

struct ComputeUniforms
{
    GLint resolutionVector;
    GLint invProjection;
    GLint cameraPos;
    GLint invView;
};
}

int main()
{
    Window window(INITIAL_RENDER_WIDTH, INITIAL_RENDER_HEIGHT);
    if (!window.initialize())
    {
        std::cerr << window.getLastError() << std::endl;
        return 1;
    }

    const auto computeShaderPath = AppPaths::findResource("computeShader.glsl");
    const auto vertexShaderPath = AppPaths::findResource("vertexShader.glsl");
    const auto fragmentShaderPath = AppPaths::findResource("fragmentShader.glsl");

    if (computeShaderPath.empty() || vertexShaderPath.empty() || fragmentShaderPath.empty())
    {
        std::cerr << "Failed to locate shader resources." << std::endl;
        return 1;
    }

    int framebufferWidth = 0;
    int framebufferHeight = 0;
    glfwGetFramebufferSize(window.p_GLFWwindow(), &framebufferWidth, &framebufferHeight);
    if (framebufferWidth <= 0 || framebufferHeight <= 0)
    {
        std::cerr << "Invalid framebuffer size reported by GLFW." << std::endl;
        return 1;
    }

    glm::ivec2 resolutionVector(framebufferWidth, framebufferHeight);
    Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));

    glfwSetWindowUserPointer(window.p_GLFWwindow(), &camera);
    glfwSetCursorPosCallback(window.p_GLFWwindow(), Camera::mouse_callback);

    Shader computeShader(computeShaderPath.string());
    Shader screenShader(vertexShaderPath.string(), fragmentShaderPath.string());

    computeShader.bind();
    const ComputeUniforms computeUniforms{
        computeShader.getUniformLocation("resolutionVector"),
        computeShader.getUniformLocation("invProjection"),
        computeShader.getUniformLocation("cameraPos"),
        computeShader.getUniformLocation("invView")
    };

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(resolutionVector.x) / static_cast<float>(resolutionVector.y),
        0.1f,
        100.0f);
    glm::mat4 invProjection = glm::inverse(projection);

    computeShader.setUniform2i(computeUniforms.resolutionVector, resolutionVector);
    computeShader.setUniformMatrix4fv(computeUniforms.invProjection, invProjection);

    Star star(&computeShader, glm::vec3(3.0f, 2.0f, -5.0f), 1.5f, glm::vec3(1.0f, 0.9f, 0.7f), 2.0f);
    BlackHole blackHole(&computeShader, glm::vec3(0.0f, 0.0f, 0.0f), star.getRadius() / 3.0f, resolutionVector.x, resolutionVector.y);

    screenShader.bind();
    screenShader.setUniform1i(screenShader.getUniformLocation("screenTexture"), 0);

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window.p_GLFWwindow()))
    {
        const float currentFrame = static_cast<float>(glfwGetTime());
        const float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window.p_GLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window.p_GLFWwindow(), true);
        }

        camera.processInput(window.p_GLFWwindow(), deltaTime);

        int currentFramebufferWidth = 0;
        int currentFramebufferHeight = 0;
        glfwGetFramebufferSize(window.p_GLFWwindow(), &currentFramebufferWidth, &currentFramebufferHeight);
        if (currentFramebufferWidth > 0 && currentFramebufferHeight > 0 &&
            (currentFramebufferWidth != resolutionVector.x || currentFramebufferHeight != resolutionVector.y))
        {
            resolutionVector = glm::ivec2(currentFramebufferWidth, currentFramebufferHeight);
            projection = glm::perspective(
                glm::radians(45.0f),
                static_cast<float>(resolutionVector.x) / static_cast<float>(resolutionVector.y),
                0.1f,
                100.0f);
            invProjection = glm::inverse(projection);

            computeShader.bind();
            computeShader.setUniform2i(computeUniforms.resolutionVector, resolutionVector);
            computeShader.setUniformMatrix4fv(computeUniforms.invProjection, invProjection);
            blackHole.resizeOutputTexture(resolutionVector.x, resolutionVector.y);
        }

        computeShader.bind();
        computeShader.setUniform3fv(computeUniforms.cameraPos, camera.getPosition());
        computeShader.setUniformMatrix4fv(computeUniforms.invView, camera.invViewMatrix());
        computeShader.dispatch((resolutionVector.x + 15) / 16, (resolutionVector.y + 15) / 16, 1);
        computeShader.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        blackHole.draw(screenShader);

        glfwSwapBuffers(window.p_GLFWwindow());
        glfwPollEvents();
    }

    return 0;
}
