#pragma once
#include <glm/glm.hpp>
#include "Ray.h"

class Camera {
public:
    glm::vec3 eye, u, v, w;
    float l, r, b, t, d;
    int nx, ny;

    Camera();
    Camera(float left, float right, float bottom, float top, float direction, int width, int height);
    Ray getRay(float i, float j);
};