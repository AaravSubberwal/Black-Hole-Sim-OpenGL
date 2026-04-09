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
    , m_presetKeyPressed{false, false, false, false}
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

    const int presetKeys[4] = {
        GLFW_KEY_1,
        GLFW_KEY_2,
        GLFW_KEY_3,
        GLFW_KEY_4
    };

    const glm::vec3 presetPositions[4] = {
        glm::vec3(6.0f, 4.0f, 6.0f),
        glm::vec3(0.0f, 0.35f, 8.0f),
        glm::vec3(0.0f, 8.0f, 0.01f),
        glm::vec3(1.2f, 0.25f, 2.0f)
    };

    for (int i = 0; i < 4; ++i)
    {
        const bool isPressed = glfwGetKey(window, presetKeys[i]) == GLFW_PRESS;
        if (isPressed && !m_presetKeyPressed[static_cast<std::size_t>(i)])
        {
            snapToView(window, presetPositions[i], glm::vec3(0.0f));
        }

        m_presetKeyPressed[static_cast<std::size_t>(i)] = isPressed;
    }
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

void Camera::snapToView(GLFWwindow *window, const glm::vec3 &position, const glm::vec3 &target)
{
    m_position = position;

    const glm::vec3 direction = glm::normalize(target - m_position);
    m_yaw = glm::degrees(atan2(direction.z, direction.x));
    m_pitch = glm::degrees(asin(glm::clamp(direction.y, -1.0f, 1.0f)));
    updateCameraVectors();

    double cursorX = 0.0;
    double cursorY = 0.0;
    glfwGetCursorPos(window, &cursorX, &cursorY);
    m_lastX = static_cast<float>(cursorX);
    m_lastY = static_cast<float>(cursorY);
    m_firstMouse = false;
}
