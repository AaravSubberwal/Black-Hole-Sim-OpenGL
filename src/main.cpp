#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Window.h"
#include "shader.h"
#include "BlackHole.h"
#include "Star.h"

const int RENDER_WIDTH = 1280;
const int RENDER_HEIGHT = 720;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = RENDER_WIDTH / 2.0f;
float lastY = RENDER_HEIGHT / 2.0f;
float yaw = -90.0f;
float pitch = 0.0f;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = ypos - lastY;  // Changed from lastY - ypos
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;  // Changed from +
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;  // Changed from -
}

int main()
{
    Window window(RENDER_WIDTH, RENDER_HEIGHT);
    glm::ivec2 resolutionVector = glm::ivec2(RENDER_WIDTH, RENDER_HEIGHT);
    
    glfwSetCursorPosCallback(window.p_GLFWwindow(), mouse_callback);
    
    Shader computeShader("res/computeShader.glsl", resolutionVector);
    Shader *p_computeShader = &computeShader;
    
    Shader screenShader("res/vertexShader.glsl", "res/fragmentShader.glsl", resolutionVector);
    
    Star star(p_computeShader, glm::vec3(3.0f, 2.0f, -5.0f), 1.5f, glm::vec3(1.0f, 0.9f, 0.7f), 2.0f);
    BlackHole blackHole(p_computeShader, glm::vec3(0.0f, 0.0f, 0.0f), star.getRadius() / 3.0f, RENDER_WIDTH, RENDER_HEIGHT);
    
    
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
    (float)window.getWidth() / (float)window.getHeight(), 
    0.1f, 100.0f);
    glm::mat4 invProjection = glm::inverse(projection);
    
    computeShader.setUniform2i("resolutionVector", resolutionVector);
    computeShader.setUniformMatrix4fv("invProjection", invProjection);

    while (!glfwWindowShouldClose(window.p_GLFWwindow()))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        processInput(window.p_GLFWwindow());
        
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 invView = glm::inverse(view);
        
        blackHole.setupCompute(computeShader);
        computeShader.setUniform3fv("cameraPos", cameraPos);
        computeShader.setUniformMatrix4fv("invView", invView);
    
        computeShader.dispatch((RENDER_WIDTH + 15) / 16, (RENDER_HEIGHT + 15) / 16, 1);
        computeShader.memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // space is black
        glClear(GL_COLOR_BUFFER_BIT);
        
        blackHole.draw(screenShader);
        
        glfwSwapBuffers(window.p_GLFWwindow());
        glfwPollEvents();
    }
    
    return 0;
}
