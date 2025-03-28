#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>

#define GLM_SWIZZLE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>



// -------------------------------------------------
// Global Variables
// -------------------------------------------------
int Width = 512;
int Height = 512;
std::vector<float> OutputImage;
// -------------------------------------------------

// 기본 벡터 클래스
struct Vec3 {
public:
	float x, y, z;
	Vec3(float x_ = 0, float y_ = 0, float z_ = 0) : x(x_), y(y_), z(z_) {}

	Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
	Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
	Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }

	float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
	Vec3 normalize() const { float len = std::sqrt(x * x + y * y + z * z); return Vec3(x / len, y / len, z / len); }
};


class Ray {
public:
	Vec3 point, direction;
	Ray(const Vec3& p, const Vec3& d) : point(p), direction(d.normalize()) {}
};

class Camera {
public:
	Vec3 eye;

	// x,y,z 방향의 단위벡터
	Vec3 u = Vec3(1, 0, 0);
	Vec3 v = Vec3(0, 1, 0);
	Vec3 w = Vec3(0, 0, 1);

	//left, right, bottom, top, direction
	float l, r, b, t, d;

	// image plain resolution
	int nx, ny;

	Camera() : eye(0, 0, 0), u(1, 0, 0), v(0, 1, 0), w(0, 0, 1),
		l(-0.1f), r(0.1f), b(-0.1f), t(0.1f), d(0.1f), nx(512), ny(512) {}

	Camera(float left, float right, float bottom, float top, float direction, int width, int height) :l(left), r(right), b(bottom), t(top), d(direction), nx(width), ny(height) {}

	Ray getRay(int i, int j) {
		float u_ = l + (r - l) * (i + 0.5f) / nx;
		float v_ = b + (t - b) * (j + 0.5f) / ny;
		Vec3 direction = (eye + u * u_ + v * v_ - w * d).normalize();;
		return Ray(eye, direction);
	}
};

// Surface 안에 재질 정보를 넣으려 하니 너무 복잡함
// 그래서 밖으로 빼고 Surface가 Material을 가지면 됨
class Material {
public:
	Vec3 ka;  // Ambient reflection coefficient
	Vec3 kd;  // Diffuse reflection coefficient
	Vec3 ks;  // Specular reflection coefficient
	float specularPower;

	Material(const Vec3& ka_, const Vec3& kd_, const Vec3& ks_, float sp)
		: ka(ka_), kd(kd_), ks(ks_), specularPower(sp) {}
};

class Surface {
public:
	Material material;

	Surface(const Material& mat) : material(mat) {}

	virtual bool intersect(const Ray& ray, float& t, Vec3& normal) const = 0;
};

class Plane : public Surface {
public:
	float y;

	Plane(float yPos, const Material& mat) : Surface(mat), y(yPos) {}

	bool intersect(const Ray& ray, float& t, Vec3& normal) const override {
		if (ray.direction.y == 0) return false;
		t = (y - ray.point.y) / ray.direction.y;
		if (t > 0) {
			normal = Vec3(0, 1, 0); // 평면의 법선은 항상 (0,1,0)
			return true;
		}
		return false;
	}
};

class Sphere : public Surface {
public:
	Vec3 center;
	float radius;

	Sphere(const Vec3& c, float r, const Material& mat) : Surface(mat), center(c), radius(r) {}

	bool intersect(const Ray& ray, float& t, Vec3& normal) const override {
		Vec3 oc = ray.point - center;
		float a = ray.direction.dot(ray.direction);
		float b = 2.0f * oc.dot(ray.direction);
		float c = oc.dot(oc) - radius * radius;
		float discriminant = b * b - 4 * a * c;

		if (discriminant < 0) return false;

		t = (-b - sqrt(discriminant)) / (2.0f * a);
		if (t > 0) {
			Vec3 hitPoint = ray.point + ray.direction * t;
			normal = (hitPoint - center).normalize();
			return true;
		}
		return false;
	}
};

Vec3 phongShading(const Vec3& point, const Vec3& normal, const Material& material, const Vec3& lightPos, const Vec3& viewPos) {
	Vec3 lightDir = (lightPos - point).normalize();
	Vec3 viewDir = (viewPos - point).normalize();
	Vec3 reflectDir = (lightDir - normal * (2 * normal.dot(lightDir))).normalize();

	// Ambient
	Vec3 ambient = material.ka;

	// Diffuse
	float diff = std::max(normal.dot(lightDir), 0.0f);
	Vec3 diffuse = material.kd * diff;

	// Specular
	float spec = pow(std::max(-viewDir.dot(reflectDir), 0.0f), material.specularPower);
	Vec3 specular = material.ks * spec;

	// Total color calculation (before gamma correction)
	return ambient + diffuse + specular;
}

// 장면(Scene) 클래스
class Scene {
public:
	std::vector<Surface*> surfaces;
	Vec3 lightPos = Vec3(-4, 4, -3);

	void addObject(Surface* obj) { surfaces.push_back(obj); }

	bool trace(const Ray& ray, float tMin, float tMax, Vec3& color) {
		Surface* closestSurface = nullptr;
		float closestT = tMax;
		Vec3 normal;

		for (Surface* obj : surfaces) {
			float t;
			Vec3 tempNormal;

			if (obj->intersect(ray, t, tempNormal) && t < closestT) {
				closestT = t;
				closestSurface = obj;
				normal = tempNormal;
			}
		}

		if (closestSurface) {
			Vec3 hitPoint = ray.point + ray.direction * closestT;

			// **그림자 검사 시작**
			Vec3 lightDir = (lightPos - hitPoint).normalize();
			Ray shadowRay(hitPoint + normal * 0.001f, lightDir);  // *법선 방향으로 살짝 이동하여 자기 자신과의 충돌 방지*

			bool inShadow = false;
			for (Surface* obj : surfaces) {
				float t;
				Vec3 tempNormal;
				if (obj->intersect(shadowRay, t, tempNormal)) {
					inShadow = true;
					break;
				}
			}

			// 그림자 내부일 경우 ambient만 적용
			if (inShadow) {
				color = closestSurface->material.ka;
			}
			else {
				color = phongShading(hitPoint, normal, closestSurface->material, lightPos, ray.point);
			}

			// 감마 보정
			color = Vec3(pow(color.x, 1.0f / 2.2f), pow(color.y, 1.0f / 2.2f), pow(color.z, 1.0f / 2.2f));
			return true;
		}
		return false;
	}
};

void render() {
	Camera camera(-0.1f, 0.1f, -0.1f, 0.1f, 0.1f, Width, Height);
	Scene scene;

	Material planeMaterial(Vec3(0.2f, 0.2f, 0.2f), Vec3(1.0f, 1.0f, 1.0f), Vec3(0, 0, 0), 0);
	Material sphere1Material(Vec3(0.2f, 0, 0), Vec3(1.0f, 0, 0), Vec3(0, 0, 0), 0);
	Material sphere2Material(Vec3(0, 0.2f, 0), Vec3(0, 0.5f, 0), Vec3(0.5f, 0.5f, 0.5f), 32);
	Material sphere3Material(Vec3(0, 0, 0.2f), Vec3(0, 0, 1.0f), Vec3(0, 0, 0), 0);

	scene.addObject(new Sphere(Vec3(-4, 0, -7), 1, sphere1Material));
	scene.addObject(new Sphere(Vec3(0, 0, -7), 2, sphere2Material));
	scene.addObject(new Sphere(Vec3(4, 0, -7), 1, sphere3Material));
	scene.addObject(new Plane(-2, planeMaterial));

	OutputImage.clear();

	for (int y = 0; y < Height; ++y) {
		for (int x = 0; x < Width; ++x) {
			Ray ray = camera.getRay(x, y);
			Vec3 color(0, 0, 0);

			if (scene.trace(ray, 0.001f, std::numeric_limits<float>::infinity(), color)) {
				OutputImage.push_back(color.x);
				OutputImage.push_back(color.y);
				OutputImage.push_back(color.z);
			}
			else {
				OutputImage.push_back(0);
				OutputImage.push_back(0);
				OutputImage.push_back(0);
			}
		}
	}
}


void resize_callback(GLFWwindow*, int nw, int nh)
{
	//This is called in response to the window resizing.
	//The new width and height are passed in so we make 
	//any necessary changes:
	Width = nw;
	Height = nh;
	//Tell the viewport to use all of our screen estate
	glViewport(0, 0, nw, nh);

	//This is not necessary, we're just working in 2d so
	//why not let our spaces reflect it?
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, static_cast<double>(Width)
		, 0.0, static_cast<double>(Height)
		, 1.0, -1.0);

	//Reserve memory for our render so that we don't do 
	//excessive allocations and render the image
	OutputImage.reserve(Width * Height * 3);
	render();
}


int main(int argc, char* argv[])
{
	// -------------------------------------------------
	// Initialize Window
	// -------------------------------------------------

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(Width, Height, "OpenGL Viewer", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	//We have an opengl context now. Everything from here on out 
	//is just managing our window or opengl directly.

	//Tell the opengl state machine we don't want it to make 
	//any assumptions about how pixels are aligned in memory 
	//during transfers between host and device (like glDrawPixels(...) )
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	//We call our resize function once to set everything up initially
	//after registering it as a callback with glfw
	glfwSetFramebufferSizeCallback(window, resize_callback);
	resize_callback(NULL, Width, Height);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		//Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// -------------------------------------------------------------
		//Rendering begins!
		glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, &OutputImage[0]);
		//and ends.
		// -------------------------------------------------------------

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		//Close when the user hits 'q' or escape
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
			|| glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}