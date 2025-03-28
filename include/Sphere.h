#pragma once
#include "Surface.h"

class Sphere : public Surface {
public:
    glm::vec3 center;
    float radius;
    Sphere(const glm::vec3& c, float r, const Material& mat);
    bool intersect(const Ray& ray, float& t, glm::vec3& normal) const override;
};