#include "App.h"

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace glender;
using namespace std;

#define GLENDER_APP_TRY try {

#ifndef NDEBUG
#define GLENDER_APP_CATCH } catch (const exception e) { \
		cerr << e.what() << endl; \
		m_shouldClose = true; \
	}
#else
#define GLENDER_APP_CATCH } catch (const exception e) { \
		cerr << e.what() << endl; \
	}
#endif


App::App(int2 windowSize, string windowName) :
			m_window(make_unique<Window>(windowSize, windowName)), m_scene(nullptr), m_renderer(nullptr), m_shouldClose(false) {
	InitGlfw();

	m_scene = make_unique<Scene>(m_window.get());
	m_renderer = make_unique<Renderer>(m_scene.get());
}

App::~App() {
	glfwTerminate();
}

void App::StartApp() {
	float currentTime = 0;
	GLENDER_APP_TRY
	GLenum error1 = glGetError();
	if (error1) {
		GlenderLog(LogTypeError, std::format("OpenGL error: {}", error1));
	}
	Start();
	m_previousTime = glfwGetTime();
	GLENDER_APP_CATCH
	while (!m_window->ShouldWindowClose() && !m_shouldClose) {
		GLENDER_APP_TRY
		currentTime = glfwGetTime();

#ifndef NDEBUG
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
#endif

		Delta = currentTime - m_previousTime;
		Tick();
		m_renderer->Render();

		m_previousTime = currentTime;

#ifndef NDEBUG
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

		m_window->PostRender();

		LateTick();

		GLenum error = glGetError();
		if (error) {
			GlenderLog(LogTypeError, std::format("OpenGL error: {}", error));
		}
		GLENDER_APP_CATCH
	}
	GLENDER_APP_TRY
	End();
	GLENDER_APP_CATCH
}

void App::Start() {
}

void App::Tick() {
}

void App::LateTick() {
}

void App::End() {
}

#ifndef NDEBUG
static void APIENTRY glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam) {
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    cout << "---------------" << endl;
    cout << "Debug message (" << id << "): " << message << endl;

    switch (source) {
    case GL_DEBUG_SOURCE_API:             cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           cout << "Source: Other"; break;
    } cout << endl;

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:               cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               cout << "Type: Other"; break;
    } cout << endl;

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:         cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: cout << "Severity: notification"; break;
    } cout << endl;
    cout << endl;
}
#endif

void App::InitGlfw() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#ifndef NDEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	m_window->InitWindow();
	
	if (!gladLoadGL(glfwGetProcAddress)) {
		cerr << "Failed to initialize GLAD" << endl;
	}

#ifndef NDEBUG
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif

	glEnable(GL_DEPTH_TEST | GL_STENCIL_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}
