#include "shader.h"

using namespace std;

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath) : isComputeShader(false)
{
    std::string vertexShader = readFromFile(vertexPath);
    std::string fragmentShader = readFromFile(fragmentPath);
    ID = createShader(vertexShader, fragmentShader);
    if (ID == 0)
    {
        std::cerr << "Failed to create shader program!" << std::endl;
    }
}

Shader::Shader(const std::string &computePath) : isComputeShader(true)
{
    std::string computeShader = readFromFile(computePath);
    ID = createComputeShader(computeShader);
    if (ID == 0)
    {
        std::cerr << "Failed to create compute shader program!" << std::endl;
    }
}

Shader::~Shader()
{
    glDeleteProgram(ID);
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
        std::cerr << "dispatch() called on non-compute shader!" << std::endl;
        return;
    }
    bind();
    glDispatchCompute(x, y, z);
}

void Shader::memoryBarrier(unsigned int barriers) const
{
    glMemoryBarrier(barriers);
}

std::string Shader::readFromFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file)
    {
        std::cerr << "Error opening file: " << path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int Shader::createShader(const string &vertexShader, const string &fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);
    if (vs == 0 || fs == 0) {
        glDeleteProgram(program);
        return 0; // fail early if compute shader didn't compile
    }
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
        glDeleteProgram(program);
        program = 0;
    }

    return program;
}

unsigned int Shader::createComputeShader(const string &computeShader)
{
    unsigned int program = glCreateProgram();
    unsigned int cs = compileShader(GL_COMPUTE_SHADER, computeShader);
    if (cs == 0) {
        glDeleteProgram(program);
        return 0; // fail early if compute shader didn't compile
    }
    glAttachShader(program, cs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(cs);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR::COMPUTE_SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
        glDeleteProgram(program);
        program = 0;
    }
    if (success)
    {
        cout << "Successfully linked compute shader program" << endl;
    }

    return program;
}

unsigned int Shader::compileShader(unsigned int type, const std::string &source)
{
    unsigned int id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = (char *)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        const char *shaderType = "unknown";
        if (type == GL_VERTEX_SHADER)
            shaderType = "vertex";
        else if (type == GL_FRAGMENT_SHADER)
            shaderType = "fragment";
        else if (type == GL_COMPUTE_SHADER)
            shaderType = "compute";
        cout << "Failed to compile " << shaderType << " shader!" << endl;
        cout << message << "\n";
        glDeleteShader(id);
        return 0;
    }
    if (result == GL_TRUE)
    {
        cout << "Successfully compiled " << (type == GL_COMPUTE_SHADER ? "compute" : type == GL_VERTEX_SHADER ? "vertex"
                                                                                                              : "fragment")
             << " shader" << endl;
    }
    return id;
}

void Shader::setUniform4f(const std::string &name, float v0, float v1, float v2, float v3)
{
    bind();
    glUniform4f(getUniformLocations(name), v0, v1, v2, v3);
}

void Shader::setUniform1i(const std::string &name, int value)
{
    bind();
    glUniform1i(getUniformLocations(name), value);
}

void Shader::setUniformMatrix4fv(const std::string &name, glm::mat4 mat, bool transpose)
{
    bind();
    glUniformMatrix4fv(getUniformLocations(name), 1, transpose, glm::value_ptr(mat));
}

void Shader::setUniform1f(const std::string &name, float value)
{
    bind();
    glUniform1f(getUniformLocations(name), value);
}

void Shader::setUniform2f(const std::string &name, float v0, float v1)
{
    bind();
    glUniform2f(getUniformLocations(name), v0, v1);
}

void Shader::setUniform3f(const std::string &name, float v0, float v1, float v2)
{
    bind();
    glUniform3f(getUniformLocations(name), v0, v1, v2);
}

void Shader::setUniform2fv(const std::string &name, const glm::vec2 &value)
{
    bind();
    glUniform2fv(getUniformLocations(name), 1, glm::value_ptr(value));
}

void Shader::setUniform3fv(const std::string &name, const glm::vec3 &value)
{
    bind();
    glUniform3fv(getUniformLocations(name), 1, glm::value_ptr(value));
}

void Shader::setUniform4fv(const std::string &name, const glm::vec4 &value)
{
    bind();
    glUniform4fv(getUniformLocations(name), 1, glm::value_ptr(value));
}

unsigned int Shader::getUniformLocations(const std::string &name)
{
    bind();
    return glGetUniformLocation(ID, name.c_str());
}