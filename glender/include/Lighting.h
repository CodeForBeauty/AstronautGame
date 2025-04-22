#pragma once

#include "Entity.h"
#include "Framebuffer.h"
#include "Material.h"
#include "Types.h"
#include "Mesh.h"
#include "Window.h"

#include <lm/lm.h>
#include <unordered_set>


namespace glender {
	static const int DIRECT_LIGHTS = 8;
	static const int POINT_LIGHTS = 100;
	static const int SPOT_LIGHTS = 50;

	struct DirectLightData;
	struct PointLightData;
	struct SpotLightData;

	struct LightsData;

	class Light : public Entity {
	public:
		Light(const lm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }, const float intensity = 1.0f, const int2 shadowResolution = { 512, 512 });
		Light(const std::vector<std::filesystem::path>& rendererShaders, const std::vector<std::filesystem::path>& remaperShaders,
			const lm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f}, const float intensity = 1.0f, const int2 shadowResolution = { 512, 512 });

		float Intensity;
		lm::vec4 Color;

		void RenderShadows(Framebuffer& sceneData, Framebuffer& output, Framebuffer& previousShadow, Geometry& geometry, const int outWidth, const int outHeight);

		virtual void GetLightData(LightsData& data) const {};

	protected:
		Framebuffer m_shadowMap;
		std::shared_ptr<Material> m_shadowRemaper;
		std::shared_ptr<Material> m_shadowMapRenderer;

		int2 m_shadowResolution;

		OverlayMesh m_quad;

		virtual void PreRender() {};
	private:

	};

	class DirectLight : public Light {
	public:
		DirectLight(const lm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }, const float intensity = 1.0f, 
			const lm::vec3 position = {0.0f, 0.0f, 0.0f}, const lm::vec3 rotation = {90.0f, 0.0f, 0.0f},
			const int2 shadowResolution = { 1024, 1024 });
		
		virtual void GetLightData(LightsData& data) const override;

	protected:

		virtual void PreRender() override;
	private:
	};

	class PointLight : public Light {
	public:
		PointLight(const float distance = 5.0f,
			const lm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }, const float intensity = 1.0f,
			const lm::vec3 position = { 0.0f, 0.0f, 0.0f }, const int2 shadowResolution = { 1024, 1024 });

		float LightDistance;

		virtual void GetLightData(LightsData& data) const override;

	protected:

		virtual void PreRender() override;
	private:

	};

	class SpotLight : public Light {
	public:
		SpotLight(const float distance = 5.0f, const float angle = 25,
			const lm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f }, const float intensity = 1.0f,
			const lm::vec3 position = { 0.0f, 0.0f, 0.0f }, const lm::vec3 rotation = {0.0f, 0.0f, 0.0f},
			const int2 shadowResolution = { 1024, 1024 });

		float LightAngle;
		float LightDistance;

		virtual void GetLightData(LightsData& data) const override;

	protected:

		virtual void PreRender() override;
	private:

	};

	struct DirectLightData {
		lm::vec4 Color;
		lm::vec4 Direction;
		float Intensity;
		float PADDING[3];
	};
	struct PointLightData {
		lm::vec4 Color;
		lm::vec4 Position;
		float Intensity;
		float Distance;
		float PADDING[2];
	};
	struct SpotLightData {
		lm::vec4 Color;
		lm::vec4 Position;
		lm::vec4 Direction;
		float Intensity;
		float InnterCone;
		float OuterCone;
		float Distance;
	};

	struct LightsData {
	public:
		DirectLightData DirectLights[DIRECT_LIGHTS];
		PointLightData PointLights[POINT_LIGHTS];
		SpotLightData SpotLights[SPOT_LIGHTS];

		int DirectLightsCount;
		int PointLightsCount;
		int SpotLightsCount;
	};
}