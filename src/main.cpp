#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Window.h"
#include "shader.h"
#include "BlackHole.h"
#include "Star.h"
#include "Camera.h"

const int RENDER_WIDTH = 1280;
const int RENDER_HEIGHT = 720;

int main()
{
    Window window(RENDER_WIDTH, RENDER_HEIGHT);
    glm::ivec2 resolutionVector = glm::ivec2(RENDER_WIDTH, RENDER_HEIGHT);
    
    Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
    
    glfwSetCursorPosCallback(window.p_GLFWwindow(), Camera::mouse_callback);
    glfwSetWindowUserPointer(window.p_GLFWwindow(), &camera);
    
    Shader computeShader("res/computeShader.glsl", resolutionVector);
    Shader *p_computeShader = &computeShader;
    
    Shader screenShader("res/vertexShader.glsl", "res/fragmentShader.glsl", resolutionVector);
    computeShader.bind();
    Star star(p_computeShader, glm::vec3(3.0f, 2.0f, -5.0f), 1.5f, glm::vec3(1.0f, 0.9f, 0.7f), 2.0f);
    BlackHole blackHole(p_computeShader, glm::vec3(0.0f, 0.0f, 0.0f), star.getRadius() / 3.0f, RENDER_WIDTH, RENDER_HEIGHT);
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
    (float)window.getWidth() / (float)window.getHeight(), 
    0.1f, 100.0f);
    glm::mat4 invProjection = glm::inverse(projection);
    
    computeShader.setUniform2i("resolutionVector", resolutionVector);
    computeShader.setUniformMatrix4fv("invProjection", invProjection);

    float lastFqrame = 0.0f;
    
    while (!glfwWindowShouldClose(window.p_GLFWwindow()))
    {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        if (glfwGetKey(window.p_GLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window.p_GLFWwindow(), true);
        
        camera.processInput(window.p_GLFWwindow(), deltaTime);
        
        computeShader.bind();
        computeShader.setUniform3fv("cameraPos", camera.getPosition());
        computeShader.setUniformMatrix4fv("invView", camera.invViewMatrix());
    
        computeShader.dispatch((RENDER_WIDTH + 15) / 16, (RENDER_HEIGHT + 15) / 16, 1);
        computeShader.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        blackHole.draw(screenShader);
        
        glfwSwapBuffers(window.p_GLFWwindow());
        glfwPollEvents();
    }
    
    return 0;
}