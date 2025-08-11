#include "Star.h"

Star::Star(glm::vec3 pos, float r, glm::vec3 color, float intens)
    : position(pos), radius(r), emissionColor(color), intensity(intens)
{
}