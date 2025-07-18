#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Framebuffer.h"
#include "Window.h"
#include "Scene.h"
#include "Renderer.h"

#include <memory>

#include "Logging.h"

namespace glender {
	class App {
	public:
		App(int2 windowSize = {800, 600}, std::string windowName = "Glender App");
		~App();

		void StartApp();

		float Delta;
	protected:
		std::unique_ptr<Window> m_window;
		std::unique_ptr<Renderer> m_renderer;
		std::unique_ptr<Scene> m_scene;

		virtual void Start();
		virtual void Tick();
		virtual void LateTick();
		virtual void End();

		bool m_shouldClose;
	private:
		void InitGlfw();
		float m_previousTime;
	};
}