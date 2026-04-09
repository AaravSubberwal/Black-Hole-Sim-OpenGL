#include "shader.h"

#include <iostream>
#include <vector>

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath)
    : ID(0), isComputeShader(false)
{
    ID = createShader(readFromFile(vertexPath), readFromFile(fragmentPath));
    if (ID == 0)
    {
        std::cerr << "Failed to create shader program." << std::endl;
    }
}

Shader::Shader(const std::string &computePath)
    : ID(0), isComputeShader(true)
{
    ID = createComputeShader(readFromFile(computePath));
    if (ID == 0)
    {
        std::cerr << "Failed to create compute shader program." << std::endl;
    }
}

Shader::~Shader()
{
    if (ID != 0)
    {
        glDeleteProgram(ID);
    }
}

void Shader::bind() const
{
    glUseProgram(ID);
}

void Shader::unbind() const
{
    glUseProgram(0);
}

void Shader::dispatch(unsigned int x, unsigned int y, unsigned int z) const
{
    if (!isComputeShader)
    {
        std::cerr << "dispatch() called on non-compute shader." << std::endl;
        return;
    }

    bind();
    glDispatchCompute(x, y, z);
}

void Shader::memoryBarrier(unsigned int barriers) const
{
    glMemoryBarrier(barriers);
}

std::string Shader::readFromFile(const std::string &path) const
{
    std::ifstream file(path);
    if (!file)
    {
        std::cerr << "Error opening file: " << path << std::endl;
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int Shader::createShader(const std::string &vertexShader, const std::string &fragmentShader) const
{
    unsigned int program = glCreateProgram();
    const unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    const unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    if (vs == 0 || fs == 0)
    {
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    int success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

unsigned int Shader::createComputeShader(const std::string &computeShader) const
{
    unsigned int program = glCreateProgram();
    const unsigned int cs = compileShader(GL_COMPUTE_SHADER, computeShader);

    if (cs == 0)
    {
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, cs);
    glLinkProgram(program);

    glDeleteShader(cs);

    int success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "ERROR::COMPUTE_SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

unsigned int Shader::compileShader(unsigned int type, const std::string &source) const
{
    const unsigned int id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> message(static_cast<std::size_t>(length > 1 ? length : 1));
        glGetShaderInfoLog(id, length, &length, message.data());

        const char *shaderType = "unknown";
        if (type == GL_VERTEX_SHADER)
        {
            shaderType = "vertex";
        }
        else if (type == GL_FRAGMENT_SHADER)
        {
            shaderType = "fragment";
        }
        else if (type == GL_COMPUTE_SHADER)
        {
            shaderType = "compute";
        }

        std::cerr << "Failed to compile " << shaderType << " shader.\n" << message.data() << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

GLint Shader::getUniformLocation(const std::string &name) const
{
    const GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cerr << "Warning: uniform '" << name << "' does not exist or is not used." << std::endl;
    }
    return location;
}

void Shader::setUniform1i(const std::string &name, int value) const
{
    setUniform1i(getUniformLocation(name), value);
}

void Shader::setUniform1i(GLint location, int value) const
{
    glUniform1i(location, value);
}

void Shader::setUniform2i(const std::string &name, const glm::ivec2 &value) const
{
    setUniform2i(getUniformLocation(name), value);
}

void Shader::setUniform2i(GLint location, const glm::ivec2 &value) const
{
    glUniform2i(location, value.x, value.y);
}

void Shader::setUniform1f(const std::string &name, float value) const
{
    setUniform1f(getUniformLocation(name), value);
}

void Shader::setUniform1f(GLint location, float value) const
{
    glUniform1f(location, value);
}

void Shader::setUniform2f(const std::string &name, float v0, float v1) const
{
    setUniform2f(getUniformLocation(name), v0, v1);
}

void Shader::setUniform2f(GLint location, float v0, float v1) const
{
    glUniform2f(location, v0, v1);
}

void Shader::setUniform3f(const std::string &name, float v0, float v1, float v2) const
{
    setUniform3f(getUniformLocation(name), v0, v1, v2);
}

void Shader::setUniform3f(GLint location, float v0, float v1, float v2) const
{
    glUniform3f(location, v0, v1, v2);
}

void Shader::setUniform4f(const std::string &name, float v0, float v1, float v2, float v3) const
{
    setUniform4f(getUniformLocation(name), v0, v1, v2, v3);
}

void Shader::setUniform4f(GLint location, float v0, float v1, float v2, float v3) const
{
    glUniform4f(location, v0, v1, v2, v3);
}

void Shader::setUniformMatrix4fv(const std::string &name, const glm::mat4 &mat, bool transpose) const
{
    setUniformMatrix4fv(getUniformLocation(name), mat, transpose);
}

void Shader::setUniformMatrix4fv(GLint location, const glm::mat4 &mat, bool transpose) const
{
    glUniformMatrix4fv(location, 1, transpose, glm::value_ptr(mat));
}

void Shader::setUniform2fv(const std::string &name, const glm::vec2 &value) const
{
    setUniform2fv(getUniformLocation(name), value);
}

void Shader::setUniform2fv(GLint location, const glm::vec2 &value) const
{
    glUniform2fv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform3fv(const std::string &name, const glm::vec3 &value) const
{
    setUniform3fv(getUniformLocation(name), value);
}

void Shader::setUniform3fv(GLint location, const glm::vec3 &value) const
{
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void Shader::setUniform4fv(const std::string &name, const glm::vec4 &value) const
{
    setUniform4fv(getUniformLocation(name), value);
}

void Shader::setUniform4fv(GLint location, const glm::vec4 &value) const
{
    glUniform4fv(location, 1, glm::value_ptr(value));
}
