#pragma once

#include "Window.h"
#include "Types.h"
#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "Events.hpp"

#include <lm/lm.h>


namespace glender {
	enum CameraEvents {
		Resize
	};

	class Camera : public Transform {
	public:
		Camera(Window* linkedWindow, int2 resolution = {800, 600}, float fov = 45, float near = 0.05f, float far = 1000.0f);

		void SetResolution(const int2& size);
		int2 GetResolution() const;
		void SetFov(const float fov);
		void SetNearFar(const float near, const float far);

		void SetRotation(const lm::vec3& rotation) override;
		void Rotate(const lm::vec3& offset) override;
		void SetRotation(const Quaternion& rotation) override;
		void Rotate(const Quaternion& offset) override;
		void SetPosition(const lm::vec3& position) override;
		void Move(const lm::vec3& offset) override;

		void SetAutoResize(const bool value);

		void ApplyPostEffects(std::shared_ptr<Texture>& texture);

		Event<CameraEvents, void(int2)> OnResize;

		Window* LinkedWindow;
	protected:
		float m_fov, m_near, m_far;
		int2 m_resolution;

		bool m_autoResize;

		lm::mat4 m_proj;
		lm::mat4 m_cam;

		lm::vec3 m_camRot;

		std::shared_ptr<Material> m_postprocMaterial;
		OverlayMesh m_postprocessing;

		void RecalculateProj();
		void UpdateBuffer();
		void RecalculateView();
	private:
		unsigned int m_uboBlock;

		void ResizeCallback(int2 size);
	};
}