#include "Lighting.h"

#include "Logging.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

using namespace glender;
using namespace std;


Light::Light(const lm::vec4 color, const float intensity, const int2 shadowResolution) : Entity(), Intensity(intensity), Color(color),
				m_shadowRemaper(make_shared<Material>(MaterialType::MaterialTypePbr,
				vector<filesystem::path> {"shaders/Quad.vert", "shaders/Shadows/DirectShadowRemap.frag"})),

				m_shadowMapRenderer(make_shared<Material>(MaterialType::MaterialTypePbr,
				vector<filesystem::path> {"shaders/Shadows/DirectShadowMap.vert", "shaders/Shadows/DirectShadowMap.frag"})),
				m_shadowResolution(shadowResolution), m_quad(m_shadowRemaper),
				m_shadowMap(shadowResolution) {
	if (shadowResolution.x <= 0 || shadowResolution.y <= 0) {
		GlenderLog(LogTypeError, format("Light shadow resolution is incorrect. Values width: {}, height: {} "
			"Values should satisfy: width > 0 && height > 0.", shadowResolution.x, shadowResolution.y));
	}

	m_shadowRemaper->SetUniform("u_ShadowMap", 0);
	m_shadowRemaper->SetUniform("u_ShadowRender", 1);

	m_shadowRemaper->SetUniform("uPosition", 2);
	m_shadowRemaper->SetUniform("uAlbedo", 3);
	m_shadowRemaper->SetUniform("uNormal", 4);
	m_shadowRemaper->SetUniform("uSMRA", 5);
}

Light::Light(const vector<filesystem::path>& rendererShaders, const vector<filesystem::path>& remaperShaders, 
		const lm::vec4 color, const float intensity, const int2 shadowResolution) : Entity(), Intensity(intensity), Color(color),
			m_shadowRemaper(make_shared<Material>(MaterialType::MaterialTypePbr, remaperShaders)),
			m_shadowMapRenderer(make_shared<Material>(MaterialType::MaterialTypePbr, rendererShaders)),
			m_shadowResolution(shadowResolution), m_quad(m_shadowRemaper),
			m_shadowMap(shadowResolution) {
	if (shadowResolution.x <= 0 || shadowResolution.y <= 0) {
		GlenderLog(LogTypeError, format("Light shadow resolution is incorrect. Values width: {}, height: {} "
			"Values should satisfy: width > 0 && height > 0.", shadowResolution.x, shadowResolution.y));
	}

	m_shadowRemaper->SetUniform("u_ShadowMap", 0);
	m_shadowRemaper->SetUniform("u_ShadowRender", 1);

	m_shadowRemaper->SetUniform("uPosition", 2);
	m_shadowRemaper->SetUniform("uAlbedo", 3);
	m_shadowRemaper->SetUniform("uNormal", 4);
	m_shadowRemaper->SetUniform("uSMRA", 5);
}


void Light::RenderShadows(Framebuffer& sceneData, Framebuffer& output, Framebuffer& previousShadow, Geometry& geometry, int outWidth, int outHeight) {
	PreRender();

	m_shadowMap.Bind();
	m_shadowMapRenderer->Bind();
	m_shadowMap.Clear();
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, m_shadowResolution.x, m_shadowResolution.y);
	geometry.Draw();
	m_shadowMap.Unbind();

	glViewport(0, 0, outWidth, outHeight);
	output.Bind();
	m_shadowMap.BindTextures();
	previousShadow.BindTextures(1);
	sceneData.BindTextures(2);
	
	m_quad.RenderMesh();
	output.Unbind();
}

DirectLight::DirectLight(const lm::vec4 color, const float intensity, const lm::vec3 position, const lm::vec3 rotation, const int2 shadowResolution) :
		Light(color, intensity, shadowResolution) {
	EntityTransform.SetPosition(position);
	EntityTransform.SetRotation(rotation);

	m_shadowMap.AddTexture(TextureDataType::TextureDataFloat, TextureType::TextureType2D, TextureUse::TextureUseDepth);

	PreRender();
}


void DirectLight::GetLightData(LightsData& data) const {
	if (data.DirectLightsCount == DIRECT_LIGHTS) {
		GlenderLog(LogTypeWarning, format("Too many direct Lights in the scene. Max supported direct lights: {}", DIRECT_LIGHTS));
		return;
	}

	data.DirectLights[data.DirectLightsCount].Color = Color;
	data.DirectLights[data.DirectLightsCount].Intensity = Intensity;
	data.DirectLights[data.DirectLightsCount].Direction = EntityTransform.GetForward();

	data.DirectLightsCount++;
}

void DirectLight::PreRender() {
	lm::mat4 light = lm::orthographic(50, -50, 50, -50, 50, 0.1f);
	light = lm::viewMatrix(EntityTransform.GetPosition() + EntityTransform.GetForward(), EntityTransform.GetPosition(), EntityTransform.GetUp()) * light;
	m_shadowMapRenderer->SetUniform("u_Light", light);
	m_shadowRemaper->SetUniform("u_Light", light);
}

PointLight::PointLight(const float distance, const lm::vec4 color, const float intensity, const lm::vec3 position, const int2 shadowResolution) :
		Light(vector<filesystem::path> {"shaders/Shadows/CubeShadowMap.vert", "shaders/Shadows/CubeShadowMap.geom", "shaders/Shadows/CubeShadowMap.frag"},
			vector<filesystem::path> {"shaders/Quad.vert", "shaders/Shadows/PointShadowRemap.frag"},
			color, intensity, shadowResolution), LightDistance(distance) {
	if (shadowResolution.x != shadowResolution.y) {
		GlenderLog(LogTypeError, format("Point light shadow resolution is incorrect. Values width: {}, height: {} "
			"Values should satisfy: width == height.", shadowResolution.x, shadowResolution.y));
	}
	EntityTransform.SetPosition(position);

	m_shadowMap.AddTexture(TextureDataType::TextureDataFloat, TextureType::TextureTypeCubemap, TextureUse::TextureUseDepth);

	PreRender();
}


void PointLight::GetLightData(LightsData& data) const {
	if (data.PointLightsCount == POINT_LIGHTS) {
		GlenderLog(LogTypeWarning, format("Too many point Lights in the scene. Max supported point lights: {}", POINT_LIGHTS));
		cerr << "Too many point Lights in the scene" << endl;
		return;
	}

	data.PointLights[data.PointLightsCount].Color = Color;
	data.PointLights[data.PointLightsCount].Intensity = Intensity;
	data.PointLights[data.PointLightsCount].Distance = LightDistance;
	data.PointLights[data.PointLightsCount].Position = EntityTransform.GetPosition();

	data.PointLightsCount++;
}

void PointLight::PreRender() {
	lm::mat4 perspective = lm::perspective(90, 0.05f, LightDistance, m_shadowResolution.x, m_shadowResolution.y);
	lm::mat4 light[6] =
	{
		lm::viewMatrix(EntityTransform.GetPosition() + lm::vec3 {-1,  0,  0}, EntityTransform.GetPosition(), {0, -1,   0})* perspective,
		lm::viewMatrix(EntityTransform.GetPosition() + lm::vec3 { 1,  0,  0}, EntityTransform.GetPosition(), {0, -1,   0})* perspective,
		lm::viewMatrix(EntityTransform.GetPosition() + lm::vec3 { 0, -1,  0}, EntityTransform.GetPosition(), {0,  0,  -1})* perspective,
		lm::viewMatrix(EntityTransform.GetPosition() + lm::vec3 { 0,  1,  0}, EntityTransform.GetPosition(), {0,  0,   1})* perspective,
		lm::viewMatrix(EntityTransform.GetPosition() + lm::vec3 { 0,  0, -1}, EntityTransform.GetPosition(), {0, -1,   0}) * perspective,
		lm::viewMatrix(EntityTransform.GetPosition() + lm::vec3 { 0,  0,  1}, EntityTransform.GetPosition(), {0, -1,   0})* perspective,
	};
	m_shadowMapRenderer->SetUniform("u_Light[0]", light[0]);
	m_shadowMapRenderer->SetUniform("u_Light[1]", light[1]);
	m_shadowMapRenderer->SetUniform("u_Light[2]", light[2]);
	m_shadowMapRenderer->SetUniform("u_Light[3]", light[3]);
	m_shadowMapRenderer->SetUniform("u_Light[4]", light[4]);
	m_shadowMapRenderer->SetUniform("u_Light[5]", light[5]);
	m_shadowMapRenderer->SetUniform("u_LightPos", EntityTransform.GetPosition());
	m_shadowMapRenderer->SetUniform("u_LightDistance", LightDistance);

	m_shadowRemaper->SetUniform("u_LightPos", EntityTransform.GetPosition());
	m_shadowRemaper->SetUniform("u_LightDistance", LightDistance);
}

SpotLight::SpotLight(const float distance, const float angle, const lm::vec4 color, const  float intensity, 
	const lm::vec3 position, const lm::vec3 rotation, const int2 shadowResolution) :
		Light(vector<filesystem::path> {"shaders/Shadows/DirectShadowMap.vert", "shaders/Shadows/DirectShadowMap.frag"},
			vector<filesystem::path> {"shaders/Quad.vert", "shaders/Shadows/SpotShadowRemap.frag"},
			color, intensity, shadowResolution), LightDistance(distance), LightAngle(angle) {
	EntityTransform.SetPosition(position);
	EntityTransform.SetRotation(rotation);

	m_shadowMap.AddTexture(TextureDataType::TextureDataFloat, TextureType::TextureType2D, TextureUse::TextureUseDepth);

	PreRender();
}


void SpotLight::GetLightData(LightsData& data) const {
	if (data.SpotLightsCount == SPOT_LIGHTS) {
		GlenderLog(LogTypeWarning, format("Too many spot Lights in the scene. Max supported spot lights: {}", SPOT_LIGHTS));
		cerr << "Too many spot Lights in the scene" << endl;
		return;
	}

	data.SpotLights[data.SpotLightsCount].Color = Color;
	data.SpotLights[data.SpotLightsCount].Intensity = Intensity;
	data.SpotLights[data.SpotLightsCount].InnterCone = cos(lm::degrees2radians(LightAngle));
	data.SpotLights[data.SpotLightsCount].OuterCone = cos(lm::degrees2radians(LightAngle + 2.0f));
	data.SpotLights[data.SpotLightsCount].Distance = LightDistance;
	data.SpotLights[data.SpotLightsCount].Direction = EntityTransform.GetForward();
	data.SpotLights[data.SpotLightsCount].Position = EntityTransform.GetPosition();
	
	data.SpotLightsCount++;
}

void SpotLight::PreRender() {
	lm::mat4 light = lm::perspective(LightAngle * 2, 0.5f, LightDistance, m_shadowResolution.x, m_shadowResolution.y);
	light = lm::viewMatrix(EntityTransform.GetPosition() - EntityTransform.GetForward(), EntityTransform.GetPosition(), EntityTransform.GetUp()) * light;
	
	m_shadowMapRenderer->SetUniform("u_Light", light);
	m_shadowRemaper->SetUniform("u_Light", light);

	m_shadowRemaper->SetUniform("u_LightPos", EntityTransform.GetPosition());
	m_shadowRemaper->SetUniform("u_LightDir", EntityTransform.GetForward());
	m_shadowRemaper->SetUniform("u_OuterCone", cos(lm::degrees2radians(LightAngle + 2.0f)));
	m_shadowRemaper->SetUniform("u_InnerCone", cos(lm::degrees2radians(LightAngle)));
}
