#pragma once

#include "Scene.h"
#include "Framebuffer.h"

#include "Lighting.h"


namespace glender {
	enum GBufferSlot {
		GBufferPosition,
		GBufferAlbedo,
		GBufferNormal,
		GBufferSMRA,
		GBufferIndex
	};

	class Renderer {
	public:
		Renderer(Scene* linkedScene);

		void Render();

		Scene* LinkedScene;

		template<typename OutType>
		OutType* GetGBufferPixel(unsigned int x, unsigned int y, GBufferSlot slot);
	
	protected:
		Framebuffer m_renderedData;
		std::shared_ptr<Material> m_shadingMaterial;
		OverlayMesh m_shadingRendering;
		std::shared_ptr<Texture> m_sceneDepth;

		Framebuffer m_renderedShadows;
		Framebuffer m_shadowsStore;
		Geometry m_meshesBatch;

		Framebuffer m_lightingBuffer;
		std::shared_ptr<Texture> m_lightingRender;

		std::shared_ptr<Material> m_skyboxMaterial;
		Geometry m_skyboxMesh;
		Cubemap m_skybox;

		std::shared_ptr<Material> m_irradianceDiffMaterial;
		Framebuffer m_sceneRadiance;
		std::shared_ptr<Texture> m_irradianceDiff;
		Geometry m_quad;

		std::shared_ptr<Material> m_irradianceSpecMaterial;
		Framebuffer m_sceneSpecRadiance;
		std::shared_ptr<Texture> m_irradianceSpec;

		std::shared_ptr<Material> m_envBrdfMaterial;
		Framebuffer m_envBrdfRender;
		std::shared_ptr<Texture> m_envBrdf;

		LightsData m_lights;

		std::unordered_map<GBufferSlot, std::shared_ptr<Texture>> m_gBuffer;
		std::unordered_map<GBufferSlot, int> m_gTexIndex;

		void RenderDiffuseIrradiance();
		void RenderSpecularIrradiance();

		void UpdateLightsBlock();
	private:
		unsigned int m_uboBlock;

	};
	template<typename OutType>
	inline OutType* Renderer::GetGBufferPixel(unsigned int x, unsigned int y, GBufferSlot slot) {
		return m_renderedData.ReadPixel<OutType>(x, y, m_gTexIndex[slot]);
	}
}