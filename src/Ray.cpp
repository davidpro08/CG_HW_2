#include "Ray.h"
#include <glm/glm.hpp>

Ray::Ray(const glm::vec3& p, const glm::vec3& d)
    : point(p), direction(glm::normalize(d)) {}