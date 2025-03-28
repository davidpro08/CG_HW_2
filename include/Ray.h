#pragma once
#include <glm/glm.hpp>

class Ray {
public:
    glm::vec3 point, direction;
    Ray(const glm::vec3& p, const glm::vec3& d);
};