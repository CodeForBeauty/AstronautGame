#include "Window.h"

#include "Logging.h"

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace glender;
using namespace std;


Window::Window(const int2 size, const string& name) : m_windowSize(size), m_windowName(name), m_glWindow(nullptr) {
	if (size.x <= 0 || size.y <= 0) {
		GlenderLog(LogTypeError, format("Window size is incorrect. Values width: {}, height: {}. "
			"Values must satisfy: width > 0 && height > 0.", size.x, size.y));
	}
}

Window::~Window() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_glWindow);
}

void Window::InitWindow() {
	m_glWindow = glfwCreateWindow(m_windowSize.x, m_windowSize.y, m_windowName.c_str(), NULL, NULL);
	if (m_glWindow == NULL) {
		GlenderLog(LogTypeError, "Failed to create GLFW window.");
		glfwTerminate();
	}
	glfwMakeContextCurrent(m_glWindow);

	glfwSetKeyCallback(m_glWindow, KeyCallback);
	glfwSetMouseButtonCallback(m_glWindow, MouseButtonCallback);
	glfwSetCursorPosCallback(m_glWindow, MousePosCallback);
	glfwSetWindowSizeCallback(m_glWindow, WindowSizeCallback);
	
	OnWindowChange.AddListener(WindowEventResize, [this](int2 size) { m_windowSize = size; });

	double xPos, yPos;
	glfwGetCursorPos(m_glWindow, &xPos, &yPos);
	m_lastMousePos = { (float)xPos, (float)yPos };

	// Init IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_glWindow, true);

	ImGui_ImplOpenGL3_Init("#version 150");
}

int2 Window::GetWindowSize() const {
	return m_windowSize;
}

void Window::SetWindowName(const string& newName) {
	m_windowName = newName;
	glfwSetWindowTitle(m_glWindow, newName.c_str());
}

void Window::SetWindowSize(const int2 size) {
	m_windowSize = size;

	glfwSetWindowSize(m_glWindow, size.x, size.y);
}

void Window::ResizeViewport() const {
	glViewport(0, 0, m_windowSize.x, m_windowSize.y);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Window::PostRender() {
	glfwSwapBuffers(m_glWindow);
	glfwPollEvents();

	for (const KeyboardKey& eventType : OnKeyboard.GetEventTypes()) {
		InputEventType type = (InputEventType)glfwGetKey(m_glWindow, eventType);
		if (type != InputEventPress) {
			continue;
		}
		OnKeyboard.Invoke(eventType, InputEventHold, InputModifierNone);
	}
	for (const MouseButton& eventType : OnMouseButton.GetEventTypes()) {
		InputEventType type = (InputEventType)glfwGetMouseButton(m_glWindow, eventType);
		if (type != InputEventPress) {
			continue;
		}
		OnMouseButton.Invoke(eventType, InputEventHold, InputModifierNone);
	}
	double xPos, yPos;
	glfwGetCursorPos(m_glWindow, &xPos, &yPos);
	m_currMousePos = { (float)xPos, (float)yPos };
	lm::vec2 diff = m_currMousePos - m_lastMousePos;
	if (diff.x != 0 || diff.y != 0) {
		OnMousePos.Invoke(MousePosEventChange, diff);
		m_lastMousePos = m_currMousePos;
	}
}

bool Window::ShouldWindowClose() {
	return glfwWindowShouldClose(m_glWindow);
}

void Window::SetCursorLock(const CursorLockType type) {
	glfwSetInputMode(m_glWindow, GLFW_CURSOR, type);
}

void glender::Window::SetWindowMode(WindowMode winMode) {
	GLFWmonitor* monitor = glfwGetWindowMonitor(m_glWindow);
	if (!monitor) {
		monitor = glfwGetPrimaryMonitor();
	}
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	switch (winMode) {
	case WindowedMode:
		glfwSetWindowMonitor(m_glWindow, GL_NONE, 0, 0, m_windowSize.x, m_windowSize.y, GLFW_DONT_CARE);
		break;
	case WindowedFullScreenMode:
		glfwSetWindowMonitor(m_glWindow, GL_NONE, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
		m_windowSize = { mode->width, mode->height };
		break;
	case FullScreenMode:
		glfwSetWindowMonitor(m_glWindow, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
		m_windowSize = { mode->width, mode->height };
		break;
	}

	OnWindowChange(WindowEventResize, m_windowSize);
}

void Window::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == InputEventHold) {
		return;
	}
	OnKeyboard.Invoke((KeyboardKey)key, (InputEventType)action, (InputEventModifiers)mods);
}

void Window::MousePosCallback(GLFWwindow* window, double xpos, double ypos) {
	OnMousePos.Invoke(MousePosEventMove, { (float)xpos, (float)ypos });
}

void Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == InputEventHold) {
		return;
	}
	OnMouseButton.Invoke((MouseButton)button, (InputEventType)action, (InputEventModifiers)mods);
}

void Window::WindowSizeCallback(GLFWwindow* window, int width, int height) {
	OnWindowChange.Invoke(WindowEventResize, { width, height });
}
