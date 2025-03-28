#pragma once
#include <glm/glm.hpp>
#include "Material.h"

glm::vec3 phongShading(const glm::vec3& point, const glm::vec3& normal,
                       const Material& material, const glm::vec3& lightPos,
                       const glm::vec3& viewPos);