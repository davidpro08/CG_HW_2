#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>
#include <random>

#define GLM_SWIZZLE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

int Width = 512;
int Height = 512;
std::vector<float> OutputImage;

const int N = 64;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> rand_offset(0.0f, 1.0f);

// --------------------------------------------
// Ray, Camera, Material, Surface 구조 정의
// --------------------------------------------

class Ray {
public:
	glm::vec3 point, direction;
	Ray(const glm::vec3& p, const glm::vec3& d) : point(p), direction(glm::normalize(d)) {}
};

class Camera {
public:
	glm::vec3 eye;
	glm::vec3 u = glm::vec3(1, 0, 0);
	glm::vec3 v = glm::vec3(0, 1, 0);
	glm::vec3 w = glm::vec3(0, 0, 1);
	float l, r, b, t, d;
	int nx, ny;

	Camera() : eye(0, 0, 0), l(-0.1f), r(0.1f), b(-0.1f), t(0.1f), d(0.1f), nx(512), ny(512) {}

	Camera(float left, float right, float bottom, float top, float direction, int width, int height)
		: l(left), r(right), b(bottom), t(top), d(direction), nx(width), ny(height) {}

	Ray getRay(float i, float j) {
		float u_ = l + (r - l) * (i + 0.5f) / nx;
		float v_ = b + (t - b) * (j + 0.5f) / ny;
		glm::vec3 dir = glm::normalize(eye + u * u_ + v * v_ - w * d);
		return Ray(eye, dir);
	}
};

class Material {
public:
	glm::vec3 ka, kd, ks;
	float specularPower;
	Material(const glm::vec3& ka_, const glm::vec3& kd_, const glm::vec3& ks_, float sp)
		: ka(ka_), kd(kd_), ks(ks_), specularPower(sp) {}
};

class Surface {
public:
	Material material;
	Surface(const Material& mat) : material(mat) {}
	virtual bool intersect(const Ray& ray, float& t, glm::vec3& normal) const = 0;
};

class Plane : public Surface {
public:
	float y;
	Plane(float yPos, const Material& mat) : Surface(mat), y(yPos) {}

	bool intersect(const Ray& ray, float& t, glm::vec3& normal) const override {
		if (ray.direction.y == 0) return false;
		t = (y - ray.point.y) / ray.direction.y;
		if (t > 0) {
			normal = glm::vec3(0, 1, 0);
			return true;
		}
		return false;
	}
};

class Sphere : public Surface {
public:
	glm::vec3 center;
	float radius;
	Sphere(const glm::vec3& c, float r, const Material& mat) : Surface(mat), center(c), radius(r) {}

	bool intersect(const Ray& ray, float& t, glm::vec3& normal) const override {
		glm::vec3 oc = ray.point - center;
		float a = glm::dot(ray.direction, ray.direction);
		float b = 2.0f * glm::dot(oc, ray.direction);
		float c = glm::dot(oc, oc) - radius * radius;
		float discriminant = b * b - 4 * a * c;
		if (discriminant < 0) return false;

		t = (-b - sqrt(discriminant)) / (2.0f * a);
		if (t > 0) {
			glm::vec3 hitPoint = ray.point + ray.direction * t;
			normal = glm::normalize(hitPoint - center);
			return true;
		}
		return false;
	}
};

glm::vec3 phongShading(const glm::vec3& point, const glm::vec3& normal, const Material& material, const glm::vec3& lightPos, const glm::vec3& viewPos) {
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

class Scene {
public:
	std::vector<Surface*> surfaces;
	glm::vec3 lightPos = glm::vec3(-4, 4, -3);

	void addObject(Surface* obj) { surfaces.push_back(obj); }

	bool trace(const Ray& ray, float tMin, float tMax, glm::vec3& color) {
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
			}
			else {
				color = phongShading(hitPoint, normal, closestSurface->material, lightPos, ray.point);
			}

			// 감마 보정
			color = glm::vec3(
				pow(color.r, 1.0f / 2.2f),
				pow(color.g, 1.0f / 2.2f),
				pow(color.b, 1.0f / 2.2f)
			);
			return true;
		}
		return false;
	}
};

glm::vec3 Antialiasing(int x, int y, Camera& camera, Scene& scene) {
	glm::vec3 accumulatedColor(0.0f);

	for (int i = 0; i < N; ++i) {
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

	return accumulatedColor / float(N);  // box filter 평균
}

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


void resize_callback(GLFWwindow*, int nw, int nh)
{
    Width = nw;
    Height = nh;
    glViewport(0, 0, nw, nh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(Width), 0.0, static_cast<double>(Height), 1.0, -1.0);
    OutputImage.reserve(Width * Height * 3);
}

int main(int argc, char* argv[])
{
    GLFWwindow* window;
    if (!glfwInit()) return -1;

    window = glfwCreateWindow(Width, Height, "OpenGL Viewer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glViewport(0, 0, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, Width, 0.0, Height, 1.0, -1.0);

    OutputImage.reserve(Width * Height * 3);

    render();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, &OutputImage[0]);
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}