#include "Plane.h"

Plane::Plane(float yPos, const Material& mat)
    : Surface(mat), y(yPos) {}

bool Plane::intersect(const Ray& ray, float& t, glm::vec3& normal) const {
    if (ray.direction.y == 0) return false;
    t = (y - ray.point.y) / ray.direction.y;
    if (t > 0) {
        normal = glm::vec3(0, 1, 0);
        return true;
    }
    return false;
}