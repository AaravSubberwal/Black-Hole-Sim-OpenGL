#pragma once
#include <vector>
#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"

class BlackHole
{
private:
    glm::vec2 position;
    float radius;
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    unsigned int segments;
    
    void generateCircle();
    void setupMesh();
    
public:
    BlackHole(glm::vec2 pos, float r, unsigned int segs = 64);
    ~BlackHole();
    
    void draw(Shader& shader);
    void setPosition(glm::vec2 pos);
    void setRadius(float r);
    glm::vec2 getPosition() const { return position; }
    float getRadius() const { return radius; }
};