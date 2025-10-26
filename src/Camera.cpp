#include "Camera.h"

Camera::Camera(glm::vec3 position)
    : m_position(position)
    , m_front(glm::vec3(0.0f, 0.0f, -1.0f))
    , m_up(glm::vec3(0.0f, 1.0f, 0.0f))
    , m_yaw(-90.0f)
    , m_pitch(0.0f)
    , m_lastX(640.0f)
    , m_lastY(360.0f)
    , m_firstMouse(true)
    , m_movementSpeed(2.5f)
    , m_mouseSensitivity(0.1f)
{
    updateCameraVectors();
}

glm::mat4 Camera::viewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::invViewMatrix() const
{
    return glm::inverse(viewMatrix());
}

void Camera::processInput(GLFWwindow* window, float deltaTime)
{
    float velocity = m_movementSpeed * deltaTime;
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_position += m_front * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_position -= m_front * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_position -= glm::normalize(glm::cross(m_front, m_up)) * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_position += glm::normalize(glm::cross(m_front, m_up)) * velocity;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        m_position -= m_up * velocity;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        m_position += m_up * velocity;
}

void Camera::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (!camera) return;
    
    if (camera->m_firstMouse)
    {
        camera->m_lastX = xpos;
        camera->m_lastY = ypos;
        camera->m_firstMouse = false;
    }
    
    float xoffset = xpos - camera->m_lastX;
    float yoffset = ypos - camera->m_lastY;
    camera->m_lastX = xpos;
    camera->m_lastY = ypos;
    
    xoffset *= camera->m_mouseSensitivity;
    yoffset *= camera->m_mouseSensitivity;
    
    camera->m_yaw += xoffset;
    camera->m_pitch += yoffset;
    
    if (camera->m_pitch > 89.0f)
        camera->m_pitch = 89.0f;
    if (camera->m_pitch < -89.0f)
        camera->m_pitch = -89.0f;
    
    camera->updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(front);
}