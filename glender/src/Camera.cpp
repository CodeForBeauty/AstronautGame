#include "Camera.h"

#include "Logging.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

using namespace glender;
using namespace std;


Camera::Camera(Window* linkedWindow, int2 resolution, float fov, float near, float far) : Transform(), m_cam(),
		m_fov(fov), m_near(near), m_far(far), m_resolution(resolution), LinkedWindow(linkedWindow), m_autoResize(false),
		m_postprocMaterial(make_shared<Material>(MaterialType::MaterialTypePbr,
			vector<filesystem::path> { "shaders/Quad.vert", "shaders/PostProcessing/BasicPostEffects.frag" })),
		m_postprocessing(m_postprocMaterial) {

	if (resolution.x <= 0 || resolution.y <= 0) {
		GlenderLog(LogTypeError, format("Camera resolution is incorrect. Values width: {}, height: {} "
			"Values should satisfy: width > 0 && height > 0.", resolution.x, resolution.y));
	}

	linkedWindow->OnWindowChange.AddListener(WindowEventResize, [this](int2 size) { ResizeCallback(size); });

	glGenBuffers(1, &m_uboBlock);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboBlock);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(lm::mat4) * 2 + sizeof(lm::vec4), NULL, GL_STATIC_DRAW);

	glBindBufferRange(GL_UNIFORM_BUFFER, 2, m_uboBlock, 0, sizeof(lm::mat4) * 2 + sizeof(lm::vec4));

	UpdateBuffer();
}

void Camera::SetResolution(const int2& size) {
	if (size.x <= 0 || size.y <= 0) {
		GlenderLog (LogTypeError, format("Camera resolution is incorrect. Values width: {}, height: {} "
			"Values should satisfy: width > 0 && height > 0.", size.x, size.y));
	}
	m_resolution = size;
	OnResize.Invoke(Resize, size);

	RecalculateProj();
}

int2 Camera::GetResolution() const {
	return m_resolution;
}

void Camera::SetFov(const float fov) {
	m_fov = fov;
	RecalculateProj();
}

void Camera::SetNearFar(const float near, const float far) {
	if ((near <= 0 || far <= 0) || far <= near) {
		GlenderLog(LogTypeError, format("Near and far values are incorrect. Values near: {}, far{}. "
			"The values should satisfy: near < far && near > 0 && far > 0.", near, far));
	}
	m_near = near;
	m_far = far;
	RecalculateProj();
}

void Camera::SetRotation(const lm::vec3& rotation) {
	m_camRot = rotation;
	Transform::SetRotation(rotation);

	UpdateBuffer();
}

void Camera::Rotate(const lm::vec3& offset) {
	m_camRot += offset;
	m_camRot = m_camRot % 360;
	Transform::SetRotation(m_camRot);

	UpdateBuffer();
}

void Camera::SetRotation(const Quaternion& rotation) {
	m_camRot = rotation.GetEuler();
	Transform::SetRotation(rotation);

	UpdateBuffer();
}

void Camera::Rotate(const Quaternion& offset) {
	m_camRot += offset.GetEuler();
	Transform::Rotate(offset);

	UpdateBuffer();
}

void Camera::SetPosition(const lm::vec3& position) {
	Transform::SetPosition(position);

	UpdateBuffer();
}

void Camera::Move(const lm::vec3& offset) {
	Transform::Move(offset);

	UpdateBuffer();
}

void Camera::SetAutoResize(bool value) {
	m_autoResize = value;
}

void Camera::ApplyPostEffects(shared_ptr<Texture>& texture) {
	texture->Bind(0);

	LinkedWindow->ResizeViewport();

	m_postprocessing.RenderMesh();
}

void Camera::RecalculateProj() {
	m_proj = lm::perspective(m_fov, m_near, m_far, m_resolution.x, m_resolution.y);
}

void Camera::UpdateBuffer() {
	RecalculateProj();
	RecalculateView();

	glBindBuffer(GL_UNIFORM_BUFFER, m_uboBlock);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lm::mat4), m_proj); // Projection
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(lm::mat4), sizeof(lm::mat4), m_cam); // View Transform
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(lm::mat4) * 2, sizeof(lm::vec4), &m_position); // View Position
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Camera::RecalculateView() {
	Quaternion rot = m_rotation;
	rot.Inverse();
	m_cam = rot.GetMatrix();
	m_cam.w = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_cam = lm::position3d(-m_position) * m_cam;
}

void Camera::ResizeCallback(int2 size) {
	if (!m_autoResize) {
		return;
	}

	m_resolution = size;
	OnResize.Invoke(Resize, size);

	UpdateBuffer();
}
