#pragma once
#include "Camera.h"
#include "Scene.h"

glm::vec3 Antialiasing(int x, int y, Camera& camera, Scene& scene, int sampleCount = 64);