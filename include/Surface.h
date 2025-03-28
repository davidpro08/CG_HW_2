#pragma once
#include "Material.h"
#include "Ray.h"

class Surface {
public:
    Material material;
    Surface(const Material& mat);
    virtual bool intersect(const Ray& ray, float& t, glm::vec3& normal) const = 0;
};