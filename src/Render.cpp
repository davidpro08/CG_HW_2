#include "Render.h"
#include "Sphere.h"
#include "Plane.h"
#include "Antialiasing.h"

int Width = 512;
int Height = 512;
std::vector<float> OutputImage;

void render() {
    Camera camera(-0.1f, 0.1f, -0.1f, 0.1f, 0.1f, Width, Height);
    Scene scene;

    scene.addObject(new Sphere(glm::vec3(-4, 0, -7), 1, Material(glm::vec3(0.2f, 0, 0), glm::vec3(1.0f, 0, 0), glm::vec3(0, 0, 0), 0)));
    scene.addObject(new Sphere(glm::vec3(0, 0, -7), 2, Material(glm::vec3(0, 0.2f, 0), glm::vec3(0, 0.5f, 0), glm::vec3(0.5f, 0.5f, 0.5f), 32)));
    scene.addObject(new Sphere(glm::vec3(4, 0, -7), 1, Material(glm::vec3(0, 0, 0.2f), glm::vec3(0, 0, 1.0f), glm::vec3(0, 0, 0), 0)));
    scene.addObject(new Plane(-2, Material(glm::vec3(0.2f), glm::vec3(1.0f), glm::vec3(0), 0)));

    OutputImage.clear();
    OutputImage.resize(Width * Height * 3);

    for (int y = 0; y < Height; ++y) {
        for (int x = 0; x < Width; ++x) {
            glm::vec3 finalColor = Antialiasing(x, y, camera, scene);
            int index = (y * Width + x) * 3;
            OutputImage[index + 0] = finalColor.r;
            OutputImage[index + 1] = finalColor.g;
            OutputImage[index + 2] = finalColor.b;
        }
    }
}