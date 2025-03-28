#include "Shader.h"
#include <glm/gtc/constants.hpp>
#include <algorithm>

glm::vec3 phongShading(const glm::vec3& point, const glm::vec3& normal,
                       const Material& material, const glm::vec3& lightPos,
                       const glm::vec3& viewPos) {
    glm::vec3 lightDir = glm::normalize(lightPos - point);
    glm::vec3 viewDir = glm::normalize(viewPos - point);
    glm::vec3 reflectDir = glm::normalize(glm::reflect(-lightDir, normal));

    glm::vec3 ambient = material.ka;
    float diff = std::max(glm::dot(normal, lightDir), 0.0f);
    glm::vec3 diffuse = material.kd * diff;

    float spec = pow(std::max(glm::dot(viewDir, reflectDir), 0.0f), material.specularPower);
    glm::vec3 specular = material.ks * spec;

    return ambient + diffuse + specular;
}