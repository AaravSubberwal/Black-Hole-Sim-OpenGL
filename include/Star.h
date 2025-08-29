#pragma once
#include <glm/glm.hpp>
#include "shader.h"

class Star
{
private:
    glm::vec3 position;
    float radius;
    glm::vec3 emissionColor;
    float intensity;
    
    Shader *p_computeShader;
public:
    Star(Shader *p_computeShader, glm::vec3 pos, float r, glm::vec3 color, float intens = 1.0f);
    ~Star() = default;

    void setupCompute(Shader computeShader);
    
    void setPosition(glm::vec3 pos) { position = pos; }
    void setRadius(float r) { radius = r; }
    void setEmissionColor(glm::vec3 color) { emissionColor = color; }
    void setIntensity(float intens) { intensity = intens; }
    
    glm::vec3 getPosition() const { return position; }
    float getRadius() const { return radius; }
    glm::vec3 getEmissionColor() const { return emissionColor; }
    float getIntensity() const { return intensity; }
};