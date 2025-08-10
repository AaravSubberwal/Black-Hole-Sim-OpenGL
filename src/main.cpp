#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Window.h"
#include "shader.h"
#include "BlackHole.h"

int main()
{
    Window window;
    
    glEnable(GL_MULTISAMPLE);
    
    Shader blackHoleShader("vertexShader.glsl", "fragmentShader.glsl");
    
    glm::mat4 projection = glm::ortho(0.0f, (float)window.getWidth(), 
                                      (float)window.getHeight(), 0.0f, 
                                      -1.0f, 1.0f);
    
    BlackHole blackhole(glm::vec2(window.getWidth() / 2.0f, window.getHeight() / 2.0f), 100.0f);
    
    while (!glfwWindowShouldClose(window.p_GLFWwindow()))
    {
        if (glfwGetKey(window.p_GLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window.p_GLFWwindow(), true);
        
        float moveSpeed = 5.0f;
        glm::vec2 pos = blackhole.getPosition();
        
        if (glfwGetKey(window.p_GLFWwindow(), GLFW_KEY_LEFT) == GLFW_PRESS)
            pos.x -= moveSpeed;
        if (glfwGetKey(window.p_GLFWwindow(), GLFW_KEY_RIGHT) == GLFW_PRESS)
            pos.x += moveSpeed;
        if (glfwGetKey(window.p_GLFWwindow(), GLFW_KEY_UP) == GLFW_PRESS)
            pos.y -= moveSpeed;
        if (glfwGetKey(window.p_GLFWwindow(), GLFW_KEY_DOWN) == GLFW_PRESS)
            pos.y += moveSpeed;
            
        blackhole.setPosition(pos);
        
        if (glfwGetKey(window.p_GLFWwindow(), GLFW_KEY_EQUAL) == GLFW_PRESS)
            blackhole.setRadius(blackhole.getRadius() + 1.0f);
        if (glfwGetKey(window.p_GLFWwindow(), GLFW_KEY_MINUS) == GLFW_PRESS)
            blackhole.setRadius(std::max(10.0f, blackhole.getRadius() - 1.0f));
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        blackHoleShader.bind();
        blackHoleShader.setUniformMatrix4fv("projection", projection);
        blackHoleShader.setUniform4f("color", 1.0f, 0.5f, 0.2f, 1.0f);
        blackhole.draw(blackHoleShader);
        
        glfwSwapBuffers(window.p_GLFWwindow());
        glfwPollEvents();
    }
    
    return 0;
}