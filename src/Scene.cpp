#include "Scene.h"
#include "Shader.h"

Scene::Scene() : lightPos(-4, 4, -3) {}

void Scene::addObject(Surface* obj) {
    surfaces.push_back(obj);
}

bool Scene::trace(const Ray& ray, float tMin, float tMax, glm::vec3& color) {
    Surface* closestSurface = nullptr;
    float closestT = tMax;
    glm::vec3 normal;

    for (Surface* obj : surfaces) {
        float t;
        glm::vec3 tempNormal;

        if (obj->intersect(ray, t, tempNormal) && t < closestT) {
            closestT = t;
            closestSurface = obj;
            normal = tempNormal;
        }
    }

    if (closestSurface) {
        glm::vec3 hitPoint = ray.point + ray.direction * closestT;
        glm::vec3 lightDir = glm::normalize(lightPos - hitPoint);
        Ray shadowRay(hitPoint + normal * 0.001f, lightDir);

        bool inShadow = false;
        for (Surface* obj : surfaces) {
            float t;
            glm::vec3 tempNormal;
            if (obj->intersect(shadowRay, t, tempNormal)) {
                inShadow = true;
                break;
            }
        }

        if (inShadow) {
            color = closestSurface->material.ka;
        } else {
            color = phongShading(hitPoint, normal, closestSurface->material, lightPos, ray.point);
        }

        color = glm::vec3(
            pow(color.r, 1.0f / 2.2f),
            pow(color.g, 1.0f / 2.2f),
            pow(color.b, 1.0f / 2.2f)
        );
        return true;
    }

    return false;
}