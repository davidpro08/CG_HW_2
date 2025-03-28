#include "Material.h"

Material::Material(const glm::vec3& ka_, const glm::vec3& kd_, const glm::vec3& ks_, float sp)
    : ka(ka_), kd(kd_), ks(ks_), specularPower(sp) {}