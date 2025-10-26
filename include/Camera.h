#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera
{
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f));
    
    glm::mat4 viewMatrix() const;
    glm::mat4 invViewMatrix() const;
    glm::vec3 getPosition() const { return m_position; }
    
    void processInput(GLFWwindow* window, float deltaTime);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    
private:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    
    float m_yaw;
    float m_pitch;
    
    float m_lastX;
    float m_lastY;
    bool m_firstMouse;
    
    float m_movementSpeed;
    float m_mouseSensitivity;
    
    void updateCameraVectors();
};