#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"

class BlackHole
{
private:
    glm::vec3 position;
    float radius;
    
    unsigned int outputTexture;
    unsigned int screenVAO, screenVBO;
    
    int textureWidth;
    int textureHeight;
    
    void createOutputTexture();
    void createScreenQuad();
    Shader *p_computeShader;
    
public:
    BlackHole(Shader *p_computeShader, glm::vec3 pos, float r, int width, int height);
    ~BlackHole();
    
    void setupCompute(Shader& computeShader);
    void draw(Shader& screenShader);
    
    void setPosition(glm::vec3 pos) { position = pos; }
    void setRadius(float r) { radius = r; }
    glm::vec3 getPosition() const { return position; }
    float getRadius() const { return radius; }
    
    unsigned int getOutputTexture() const { return outputTexture; }
};