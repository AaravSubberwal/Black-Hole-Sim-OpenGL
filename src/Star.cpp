#include "Star.h"

Star::Star(Shader *p_computeShader, glm::vec3 pos, float r, glm::vec3 color, float intens)
    : position(pos), radius(r), emissionColor(color), intensity(intens),p_computeShader(p_computeShader)
{   
    p_computeShader->setUniform3fv("starCenter", position);
    p_computeShader->setUniform1f("starRadius", radius);
    p_computeShader->setUniform3fv("starEmissionColor", emissionColor);
    p_computeShader->setUniform1f("starIntensity", intensity);
}