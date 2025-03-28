#include "Camera.h"

Camera::Camera() : eye(0,0,0), u(1,0,0), v(0,1,0), w(0,0,1),
    l(-0.1f), r(0.1f), b(-0.1f), t(0.1f), d(0.1f), nx(512), ny(512) {}

Camera::Camera(float left, float right, float bottom, float top, float direction, int width, int height)
    : l(left), r(right), b(bottom), t(top), d(direction), nx(width), ny(height), eye(0,0,0), u(1,0,0), v(0,1,0), w(0,0,1) {}

Ray Camera::getRay(float i, float j) {
    float u_ = l + (r - l) * (i + 0.5f) / nx;
    float v_ = b + (t - b) * (j + 0.5f) / ny;
    glm::vec3 dir = glm::normalize(eye + u * u_ + v * v_ - w * d);
    return Ray(eye, dir);
}