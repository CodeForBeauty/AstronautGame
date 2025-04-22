#include "Renderer.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

using namespace glender;
using namespace std;


Renderer::Renderer(Scene* linkedScene) : 
		LinkedScene(linkedScene), m_uboBlock(GL_NONE), m_sceneDepth(nullptr), m_lightingRender(nullptr),
		m_renderedShadows(linkedScene->SceneCamera.GetResolution()),
		m_renderedData(linkedScene->SceneCamera.GetResolution()),
		m_shadowsStore(linkedScene->SceneCamera.GetResolution()),
		m_lightingBuffer(linkedScene->SceneCamera.GetResolution()),

		m_shadingMaterial(make_shared<Material>(MaterialType::MaterialTypePbr,
			vector<filesystem::path> { "shaders/Quad.vert", "shaders/Shading/PBR.frag" })),

		m_skyboxMaterial(make_shared<Material>(MaterialType::MaterialTypePbr,
			vector<filesystem::path> { "shaders/Skyboxes/Skybox.vert", "shaders/Skyboxes/Skybox.frag" })),

		m_skyboxMesh({ 
			{ {-1,  1,  1} }, // 0
			{ { 1,  1,  1} }, // 1
			{ { 1, -1,  1} }, // 2
			{ {-1, -1,  1} }, // 3

			{ {-1,  1, -1} }, // 4
			{ { 1,  1, -1} }, // 5
			{ { 1, -1, -1} }, // 6
			{ {-1, -1, -1} }, // 7
			}, {},
			{
				0, 1, 2,
				2, 3, 0,

				6, 5, 4,
				4, 7, 6,

				1, 5, 6,
				6, 2, 1,

				4, 0, 3,
				3, 7, 4,

				4, 5, 1,
				1, 0, 4,

				3, 2, 6,
				6, 7, 3
			}
		),
		m_shadingRendering(m_shadingMaterial), 
		m_skybox({ 
			/*{CubemapFront, "Resources/Skyboxes/YellowCloudy/yellowcloud_ft.jpg"},
			{CubemapBack, "Resources/Skyboxes/YellowCloudy/yellowcloud_bk.jpg"},
			{CubemapRight, "Resources/Skyboxes/YellowCloudy/yellowcloud_rt.jpg"},
			{CubemapLeft, "Resources/Skyboxes/YellowCloudy/yellowcloud_lf.jpg"},
			{CubemapTop, "Resources/Skyboxes/YellowCloudy/yellowcloud_up.jpg"},
			{CubemapBottom, "Resources/Skyboxes/YellowCloudy/yellowcloud_dn.jpg"},*/

			/*{CubemapFront, "Resources/Skyboxes/AboveWater/skyrender0001.bmp"},
			{CubemapBack, "Resources/Skyboxes/AboveWater/skyrender0004.bmp"},
			{CubemapRight, "Resources/Skyboxes/AboveWater/skyrender0005.bmp"},
			{CubemapLeft, "Resources/Skyboxes/AboveWater/skyrender0002.bmp"},
			{CubemapTop, "Resources/Skyboxes/AboveWater/skyrender0003.bmp"},
			{CubemapBottom, "Resources/Skyboxes/AboveWater/skyrender0006.bmp"},*/

			{CubemapFront, "Resources/Skyboxes/Night/front.png"},
			{CubemapBack, "Resources/Skyboxes/Night/back.png"},
			{CubemapRight, "Resources/Skyboxes/Night/right.png"},
			{CubemapLeft, "Resources/Skyboxes/Night/left.png"},
			{CubemapTop, "Resources/Skyboxes/Night/top.png"},
			{CubemapBottom, "Resources/Skyboxes/Night/bottom.png"},

			/*{CubemapFront, "Resources/Skyboxes/InSnow/front.bmp"},
			{CubemapBack, "Resources/Skyboxes/InSnow/back.bmp"},
			{CubemapRight, "Resources/Skyboxes/InSnow/right.bmp"},
			{CubemapLeft, "Resources/Skyboxes/InSnow/left.bmp"},
			{CubemapTop, "Resources/Skyboxes/InSnow/top.bmp"},
			{CubemapBottom, "Resources/Skyboxes/InSnow/bottom.bmp"},*/
		}),

		m_irradianceDiffMaterial(make_shared<Material>(MaterialType::MaterialTypePbr,
			vector<filesystem::path> { "shaders/Skyboxes/Irradiance.vert", "shaders/Skyboxes/DiffuseIrradiance.frag", 
				"shaders/Skyboxes/Irradiance.geom" })),

		m_irradianceDiff(nullptr),
		m_sceneRadiance(int2{512, 512}),
		m_quad({
			{{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
			{{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
			{{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
			{{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}}
				},
			{
				{{1.0f, 1.0f}},
				{{0.0f, 1.0f}},
				{{0.0f, 0.0f}},
				{{1.0f, 0.0f}},
			},
			{
			0, 1, 2,
			2, 3, 0
			}),

		m_irradianceSpecMaterial(make_shared<Material>(MaterialType::MaterialTypePbr,
			vector<filesystem::path> { "shaders/Skyboxes/Irradiance.vert", "shaders/Skyboxes/SpecularIrradiance.frag",
			"shaders/Skyboxes/Irradiance.geom" })),

		m_irradianceSpec(nullptr),
		m_sceneSpecRadiance(int2{ 256, 256 }),

		m_envBrdfMaterial(make_shared<Material>(MaterialType::MaterialTypePbr,
			vector<filesystem::path> { "shaders/Skyboxes/Irradiance.vert", "shaders/Skyboxes/BRDF.frag" })),

		m_envBrdf(nullptr),
		m_envBrdfRender(int2{ 512, 512 }) {

	linkedScene->SceneCamera.OnResize.AddListener(Resize, [this](int2 size) {
		m_renderedData.Resize(size);
		m_shadowsStore.Resize(size);
		m_renderedShadows.Resize(size);
		m_lightingBuffer.Resize(size);
	});

	int2 viewSize = linkedScene->SceneCamera.GetResolution();
	shared_ptr<Texture> gBuffer = nullptr;
	m_gBuffer[GBufferPosition] = m_renderedData.AddTexture(TextureDataFloat); // Position
	m_gBuffer[GBufferAlbedo] = m_renderedData.AddTexture(TextureDataUByte);  // Albedo
	m_gBuffer[GBufferNormal] = m_renderedData.AddTexture(TextureDataFloat); // Normal
	m_gBuffer[GBufferSMRA] = m_renderedData.AddTexture(TextureDataFloat); // Specular, Metalic, Roughness, AO
	m_gBuffer[GBufferIndex] = m_renderedData.AddTexture(TextureDataInt, TextureType2D, TextureUseGrayscale); // Object index
	m_sceneDepth = m_renderedData.AddTexture(TextureDataFloat, TextureType2D, TextureUseDepth); // Depth

	m_gTexIndex[GBufferPosition] = 0;
	m_gTexIndex[GBufferAlbedo] = 1;
	m_gTexIndex[GBufferNormal] = 2;
	m_gTexIndex[GBufferSMRA] = 3;
	m_gTexIndex[GBufferIndex] = 4;

	m_lightingBuffer.AddTexture(m_sceneDepth);

	m_lightingRender = m_lightingBuffer.AddTexture(TextureDataUByte);

	m_shadingMaterial->SetUniform("uPosition", 0);
	m_shadingMaterial->SetUniform("uAlbedo", 1);
	m_shadingMaterial->SetUniform("uNormal", 2);
	m_shadingMaterial->SetUniform("uSMRA", 3);

	m_renderedShadows.AddTexture(TextureDataFloat, TextureType2D, TextureUseColor);

	m_shadowsStore.AddTexture(TextureDataFloat, TextureType2D, TextureUseColor);

	glGenBuffers(1, &m_uboBlock);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboBlock);

	glBufferData(GL_UNIFORM_BUFFER, sizeof(LightsData), NULL, GL_DYNAMIC_DRAW);

	glBindBufferRange(GL_UNIFORM_BUFFER, 3, m_uboBlock, 0, sizeof(LightsData));
	m_shadingMaterial->UniformBlockBinding("Lights", 3);

	m_shadingMaterial->SetUniform("u_Shadow", 4);

	m_shadingMaterial->SetUniform("u_Irradiance", 5);

	m_shadingMaterial->SetUniform("u_IrradianceSpec", 6);
	m_shadingMaterial->SetUniform("u_Brdf", 7);

	// Irradiance
	RenderDiffuseIrradiance();
	RenderSpecularIrradiance();
}

void Renderer::Render() {
	int2 viewSize = LinkedScene->SceneCamera.GetResolution();
	glViewport(0, 0, viewSize.x, viewSize.y);
	// G buffer
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	m_renderedData.Bind();
	m_renderedData.Clear();

	vector<Vertex> verticesDyn;
	vector<VertexStatic> verticesSt;
	vector<unsigned int> indices;

	for (int i = 0; i < LinkedScene->SceneMeshes.size(); i++) {
		LinkedScene->SceneMeshes[i]->SceneIndex = i + 1;

		LinkedScene->SceneMeshes[i]->RenderMesh();

		LinkedScene->SceneMeshes[i]->GetVertices(verticesDyn, verticesSt, indices, verticesDyn.size());
	}

	m_renderedData.Unbind();
	// Shadows
	m_meshesBatch.SetGeometry(verticesDyn, verticesSt, indices);
	m_renderedShadows.Bind();
	m_renderedShadows.Clear({1, 1, 1, 1});

	m_shadowsStore.Bind();
	m_shadowsStore.Clear({ 0, 0, 0, 1 });

	for (int i = 0; i < LinkedScene->SceneLights.size(); i++) {
		LinkedScene->SceneLights[i]->RenderShadows(m_renderedData, m_renderedShadows, m_shadowsStore, m_meshesBatch,
			viewSize.x, viewSize.y);

		m_shadowsStore.BlitFrom(m_renderedShadows);
	}

	m_renderedData.Unbind();
	// Lighting
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	m_lightingBuffer.Bind();
	m_lightingBuffer.Clear();
	m_renderedData.BindTextures();

	m_renderedShadows.BindTextures(4);

	m_irradianceDiff->Bind(5);
	m_irradianceSpec->Bind(6);
	m_envBrdf->Bind(7);

	UpdateLightsBlock();
	m_shadingRendering.RenderMesh();

	glEnable(GL_DEPTH_TEST);
	m_skybox.Bind(0);
	//m_irradianceDiff->Bind(0);
	//m_irradianceSpec->Bind(0);
	m_skyboxMaterial->Bind();
	m_skyboxMesh.Draw();

	m_lightingBuffer.Unbind();

	// Post processing
	glDisable(GL_DEPTH_TEST);
	LinkedScene->SceneWindow->ResizeViewport();
	LinkedScene->SceneCamera.ApplyPostEffects(m_lightingRender);
}

void Renderer::RenderDiffuseIrradiance() {
	m_irradianceDiff = m_sceneRadiance.AddTexture(TextureDataUByte, TextureTypeCubemap, TextureUseColor);

	m_irradianceDiff->SetTextureWrap(TextureWrapClamp);

	lm::mat4 sides[6] =
	{
		lm::viewMatrix(lm::vec3 {-1,  0,  0}, {0, 0, 0}, {0, -1,   0}),
		lm::viewMatrix(lm::vec3 { 1,  0,  0}, {0, 0, 0}, {0, -1,   0}),
		lm::viewMatrix(lm::vec3 { 0,  1,  0}, {0, 0, 0}, {0,  0,  -1}),
		lm::viewMatrix(lm::vec3 { 0, -1,  0}, {0, 0, 0}, {0,  0,   1}),
		lm::viewMatrix(lm::vec3 { 0,  0, -1}, {0, 0, 0}, {0, -1,   0}),
		lm::viewMatrix(lm::vec3 { 0,  0,  1}, {0, 0, 0}, {0, -1,   0}),
	};
	m_irradianceDiffMaterial->SetUniform("u_Sides[0]", sides[0]);
	m_irradianceDiffMaterial->SetUniform("u_Sides[1]", sides[1]);
	m_irradianceDiffMaterial->SetUniform("u_Sides[2]", sides[2]);
	m_irradianceDiffMaterial->SetUniform("u_Sides[3]", sides[3]);
	m_irradianceDiffMaterial->SetUniform("u_Sides[4]", sides[4]);
	m_irradianceDiffMaterial->SetUniform("u_Sides[5]", sides[5]);

	m_irradianceDiffMaterial->SetUniform("u_Skybox", 0);
	glDisable(GL_DEPTH_TEST);
	m_skybox.Bind(0);
	m_sceneRadiance.Bind();
	m_irradianceDiffMaterial->Bind();

	m_quad.Draw();

	m_skybox.Unbind();
	m_sceneRadiance.Unbind();
	m_irradianceDiffMaterial->Unbind();
	glEnable(GL_DEPTH_TEST);
}

void Renderer::RenderSpecularIrradiance() {
	m_irradianceSpec = m_sceneSpecRadiance.AddTexture(TextureDataUByte, TextureTypeCubemap, TextureUseColor);

	m_irradianceSpec->SetTextureWrap(TextureWrapClamp);
	m_irradianceSpec->SetTextureMinFilter(TextureMinFilterLinearMipLinear);
	m_irradianceSpec->GenerateMipmaps();

	lm::mat4 sides[6] =
	{
		lm::viewMatrix(lm::vec3 {-1,  0,  0}, {0, 0, 0}, {0, -1,   0}),
		lm::viewMatrix(lm::vec3 { 1,  0,  0}, {0, 0, 0}, {0, -1,   0}),
		lm::viewMatrix(lm::vec3 { 0,  1,  0}, {0, 0, 0}, {0,  0,  -1}),
		lm::viewMatrix(lm::vec3 { 0, -1,  0}, {0, 0, 0}, {0,  0,   1}),
		lm::viewMatrix(lm::vec3 { 0,  0, -1}, {0, 0, 0}, {0, -1,   0}),
		lm::viewMatrix(lm::vec3 { 0,  0,  1}, {0, 0, 0}, {0, -1,   0}),
	};
	m_irradianceSpecMaterial->SetUniform("u_Sides[0]", sides[0]);
	m_irradianceSpecMaterial->SetUniform("u_Sides[1]", sides[1]);
	m_irradianceSpecMaterial->SetUniform("u_Sides[2]", sides[2]);
	m_irradianceSpecMaterial->SetUniform("u_Sides[3]", sides[3]);
	m_irradianceSpecMaterial->SetUniform("u_Sides[4]", sides[4]);
	m_irradianceSpecMaterial->SetUniform("u_Sides[5]", sides[5]);

	m_irradianceSpecMaterial->SetUniform("u_Skybox", 0);
	glDisable(GL_DEPTH_TEST);
	m_sceneSpecRadiance.Bind();

	unsigned int maxMipLevels = 5;
	int2 size = m_sceneSpecRadiance.GetSize();
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
		unsigned int mipWidth = size.x * pow(0.5, mip);
		unsigned int mipHeight = size.y * pow(0.5, mip);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		m_irradianceSpecMaterial->SetUniform("u_Roughness", roughness);

		m_irradianceSpecMaterial->Bind();
		m_sceneSpecRadiance.ChangeMipLevel(m_irradianceSpec, mip);
		m_skybox.Bind(0);
		m_quad.Draw();
	}

	m_skybox.Unbind();
	m_sceneSpecRadiance.Unbind();
	m_irradianceSpecMaterial->Unbind();
	glEnable(GL_DEPTH_TEST);

	// BRDF
	m_envBrdf = m_envBrdfRender.AddTexture(TextureDataFloat);
	m_envBrdf->SetTextureWrap(TextureWrapClamp);
	
	m_envBrdfRender.Bind();
	m_envBrdfMaterial->Bind();

	m_quad.Draw();

	m_envBrdfRender.Unbind();
	m_envBrdfMaterial->Unbind();
}

void Renderer::UpdateLightsBlock() {
	m_lights.DirectLightsCount = 0;
	m_lights.PointLightsCount = 0;
	m_lights.SpotLightsCount = 0;

	for (int i = 0; i < LinkedScene->SceneLights.size(); i++) {
		LinkedScene->SceneLights[i]->GetLightData(m_lights);
	}

	glBindBuffer(GL_UNIFORM_BUFFER, m_uboBlock);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightsData), &m_lights);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
