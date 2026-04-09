#include "BlackHole.h"

BlackHole::BlackHole(Shader *p_computeShader, glm::vec3 pos, float r, int width, int height) 
    : position(pos), radius(r), outputTexture(0), screenVAO(0), screenVBO(0), textureWidth(width), textureHeight(height)
{
    createOutputTexture();
    createScreenQuad();
    
    p_computeShader->setUniform3fv("bh_center", position);
    p_computeShader->setUniform1f("Rs", radius);
    p_computeShader->setUniform1f("diskInnerRadius", radius * 3.0f);     // ISCO for Schwarzschild
    p_computeShader->setUniform1f("diskOuterRadius", radius * 20.0f);    
    p_computeShader->setUniform3fv("diskColor", glm::vec3(1.0f, 0.3f, 0.05f)); // orange-red
    p_computeShader->setUniform1f("diskIntensity", 2.0f);
    p_computeShader->setUniform3fv("diskNormal", glm::vec3(0.0f, 1.0f, 0.0f)); // horizontal disk
    p_computeShader->setUniform1f("diskBetaInner", 0.42f);
    p_computeShader->setUniform1f("diskBetaMax", 0.45f);
    p_computeShader->setUniform1f("dopplerStrength", 0.85f);
    p_computeShader->setUniform1i("enableDopplerBeaming", 1);

    glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

BlackHole::~BlackHole()
{
    glDeleteTextures(1, &outputTexture);
    glDeleteVertexArrays(1, &screenVAO);
    glDeleteBuffers(1, &screenVBO);
}

void BlackHole::createOutputTexture()
{
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void BlackHole::resizeOutputTexture(int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        return;
    }

    if (textureWidth == width && textureHeight == height)
    {
        return;
    }

    textureWidth = width;
    textureHeight = height;

    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureWidth, textureHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindImageTexture(0, outputTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

void BlackHole::createScreenQuad()
{
    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &screenVAO);
    glGenBuffers(1, &screenVBO);
    
    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void BlackHole::draw(Shader& screenShader)
{
    screenShader.bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    
    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
