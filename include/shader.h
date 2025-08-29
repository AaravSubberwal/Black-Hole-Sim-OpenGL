#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

class Shader
{
private:
    unsigned int ID;
    bool isComputeShader;

    glm::ivec2 resolutionVector;

    unsigned int getUniformLocations(const std::string &name);
    std::string readFromFile(const std::string &path);
    
    unsigned int compileShader(unsigned int type, const std::string &source);
    unsigned int createShader(const std::string &vertexShader, const std::string &fragmentShader);
    unsigned int createComputeShader(const std::string &computeShader);
    
public:
    Shader(const std::string &vertexPath, const std::string &fragmentPath, glm::ivec2 resolutionVector);
    Shader(const std::string &computePath, glm::ivec2 resolutionVector);
    ~Shader();

    void bind() const;
    void unbind() const;
    void dispatch(unsigned int x, unsigned int y, unsigned int z) const;
    void memoryBarrier(unsigned int barriers) const;

    void setUniform1i(const std::string &name, int value);
    void setUniform2i(const std::string &name, const glm::ivec2 &value);
    void setUniform1f(const std::string &name, float value);
    void setUniform2f(const std::string &name, float v0, float v1);
    void setUniform3f(const std::string &name, float v0, float v1, float v2);
    void setUniform4f(const std::string &name, float v0, float v1, float v2, float v3);
    void setUniformMatrix4fv(const std::string &name, glm::mat4 mat, bool transpose = GL_FALSE);
    
    void setUniform2fv(const std::string &name, const glm::vec2 &value);
    void setUniform3fv(const std::string &name, const glm::vec3 &value);
    void setUniform4fv(const std::string &name, const glm::vec4 &value);
    
    unsigned int getID() const { return ID; }
    bool isCompute() const { return isComputeShader; }
};