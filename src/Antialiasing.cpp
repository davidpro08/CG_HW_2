#include "Antialiasing.h"
#include <random>

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> rand_offset(0.0f, 1.0f);

glm::vec3 Antialiasing(int x, int y, Camera& camera, Scene& scene, int sampleCount) {
    glm::vec3 accumulatedColor(0.0f);

    for (int i = 0; i < sampleCount; ++i) {
        float dx = rand_offset(gen);
        float dy = rand_offset(gen);

        float sampleX = x + dx;
        float sampleY = y + dy;

        Ray ray = camera.getRay(sampleX, sampleY);
        glm::vec3 sampleColor(0.0f);

        if (scene.trace(ray, 0.001f, std::numeric_limits<float>::infinity(), sampleColor)) {
            accumulatedColor += sampleColor;
        }
    }

    return accumulatedColor / float(sampleCount);
}