#include "BlackHole.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

BlackHole::BlackHole(glm::vec2 pos, float r, unsigned int segs)
    : position(pos), radius(r), segments(segs), VAO(0), VBO(0), EBO(0)
{
    generateCircle();
    setupMesh();
}

BlackHole::~BlackHole()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void BlackHole::generateCircle()
{
    vertices.clear();
    indices.clear();

    vertices.push_back(position.x);
    vertices.push_back(position.y);
    vertices.push_back(0.0f);

    float angleStep = 2.0f * M_PI / segments;
    for (unsigned int i = 0; i <= segments; i++)
    {
        float angle = i * angleStep;
        float x = position.x + radius * cos(angle);
        float y = position.y + radius * sin(angle);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.0f);
    }

    for (unsigned int i = 1; i <= segments; i++)
    {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
}

void BlackHole::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void BlackHole::draw(Shader &shader)
{
    shader.bind();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void BlackHole::setPosition(glm::vec2 pos)
{
    position = pos;
    generateCircle();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BlackHole::setRadius(float r)
{
    radius = r;
    generateCircle();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}