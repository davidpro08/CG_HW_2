#pragma once
#include "Surface.h"

class Plane : public Surface {
public:
    float y;
    Plane(float yPos, const Material& mat);
    bool intersect(const Ray& ray, float& t, glm::vec3& normal) const override;
};