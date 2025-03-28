#pragma once
#include <glm/glm.hpp>

class Material {
public:
    glm::vec3 ka, kd, ks;
    float specularPower;
    Material(const glm::vec3& ka_, const glm::vec3& kd_, const glm::vec3& ks_, float sp);
};