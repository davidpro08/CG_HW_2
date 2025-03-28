#pragma once
#include <vector>
#include "Surface.h"
#include "Ray.h"

class Scene {
public:
    std::vector<Surface*> surfaces;
    glm::vec3 lightPos;

    Scene();
    void addObject(Surface* obj);
    bool trace(const Ray& ray, float tMin, float tMax, glm::vec3& color);
};