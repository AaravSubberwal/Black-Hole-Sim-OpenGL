#pragma once

#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <sstream>
#include <string>

class Shader
{
private:
    unsigned int ID;
    bool isComputeShader;

    std::string readFromFile(const std::string &path) const;
    unsigned int compileShader(unsigned int type, const std::string &source) const;
    unsigned int createShader(const std::string &vertexShader, const std::string &fragmentShader) const;
    unsigned int createComputeShader(const std::string &computeShader) const;

public:
    Shader(const std::string &vertexPath, const std::string &fragmentPath);
    explicit Shader(const std::string &computePath);
    ~Shader();

    void bind() const;
    void unbind() const;
    void dispatch(unsigned int x, unsigned int y, unsigned int z) const;
    void memoryBarrier(unsigned int barriers) const;

    GLint getUniformLocation(const std::string &name) const;

    void setUniform1i(const std::string &name, int value) const;
    void setUniform1i(GLint location, int value) const;
    void setUniform2i(const std::string &name, const glm::ivec2 &value) const;
    void setUniform2i(GLint location, const glm::ivec2 &value) const;
    void setUniform1f(const std::string &name, float value) const;
    void setUniform1f(GLint location, float value) const;
    void setUniform2f(const std::string &name, float v0, float v1) const;
    void setUniform2f(GLint location, float v0, float v1) const;
    void setUniform3f(const std::string &name, float v0, float v1, float v2) const;
    void setUniform3f(GLint location, float v0, float v1, float v2) const;
    void setUniform4f(const std::string &name, float v0, float v1, float v2, float v3) const;
    void setUniform4f(GLint location, float v0, float v1, float v2, float v3) const;
    void setUniformMatrix4fv(const std::string &name, const glm::mat4 &mat, bool transpose = GL_FALSE) const;
    void setUniformMatrix4fv(GLint location, const glm::mat4 &mat, bool transpose = GL_FALSE) const;
    void setUniform2fv(const std::string &name, const glm::vec2 &value) const;
    void setUniform2fv(GLint location, const glm::vec2 &value) const;
    void setUniform3fv(const std::string &name, const glm::vec3 &value) const;
    void setUniform3fv(GLint location, const glm::vec3 &value) const;
    void setUniform4fv(const std::string &name, const glm::vec4 &value) const;
    void setUniform4fv(GLint location, const glm::vec4 &value) const;

    unsigned int getID() const { return ID; }
    bool isCompute() const { return isComputeShader; }
};
