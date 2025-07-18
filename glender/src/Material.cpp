#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Material.h"

#include "Logging.h"

#include <fstream>
#include <iostream>

using namespace glender;
using namespace std;


Material::Material(const MaterialType type, const vector<shared_ptr<Shader>>& shaders) : Type(type), m_shaders(), m_textures(), m_shaderProgram(0) {
	LoadShaders(shaders);
}

Material::Material(const MaterialType type, const vector<filesystem::path>& shaderPaths) : Type(type), m_shaders(), m_textures(), m_shaderProgram(0) {
	LoadShaders(shaderPaths);
}

Material::~Material() {
	glDeleteProgram(m_shaderProgram);
}

void glender::Material::BindMaterial() {
	if (m_shaderProgram == GL_NONE) {
		GlenderLog(LogTypeError, "Material unitialized.");
		return;
	}
	glUseProgram(m_shaderProgram);
}

void Material::Bind() {
	BindMaterial();

	for (pair<const TextureSlot, shared_ptr<Texture>>& texture : m_textures) {
		texture.second->Bind(texture.first);
	}
}

void Material::Unbind() {
	glUseProgram(GL_NONE);

	for (pair<const TextureSlot, shared_ptr<Texture>>& texture : m_textures) {
		texture.second->Unbind();
	}
}

void Material::LoadShaders(const vector<shared_ptr<Shader>>& shaders) {
	for (int i = 0; i < m_shaders.size(); i++) {
		m_shaders[shaders[i]->GetType()] = shaders[i];
		shaders[i]->CompileShader();
	}

	Recompile();
}

void Material::LoadShaders(const vector<filesystem::path>& shaderPaths) {
	for (filesystem::path file : shaderPaths) {
		shared_ptr<Shader> shader = make_shared<Shader>(file);
		m_shaders[shader->GetType()] = shader;
	}

	Recompile();
}

bool Material::SetUniform(const char* name, const int value) {
	BindMaterial();
	if (glGetUniformLocation(m_shaderProgram, name) == -1)
		return false;
	glUniform1i(glGetUniformLocation(m_shaderProgram, name), value);
	glUseProgram(GL_NONE);
	return true;
}

bool Material::SetUniform(const char* name, const float value) {
	BindMaterial();
	if (glGetUniformLocation(m_shaderProgram, name) == -1)
		return false;
	glUniform1f(glGetUniformLocation(m_shaderProgram, name), value);
	glUseProgram(GL_NONE);
	return true;
}

bool Material::SetUniform(const char* name, const bool value) {
	BindMaterial();
	if (glGetUniformLocation(m_shaderProgram, name) == -1)
		return false;
	glUniform1i(glGetUniformLocation(m_shaderProgram, name), value);
	glUseProgram(GL_NONE);
	return true;
}

bool Material::SetUniform(const char* name, lm::mat2 value) {
	BindMaterial();
	if (glGetUniformLocation(m_shaderProgram, name) == -1)
		return false;
	glUniformMatrix2fv(glGetUniformLocation(m_shaderProgram, name), 1, GL_FALSE, value);
	glUseProgram(GL_NONE);
	return true;
}

bool Material::SetUniform(const char* name, lm::mat3 value) {
	BindMaterial();
	if (glGetUniformLocation(m_shaderProgram, name) == -1)
		return false;
	glUniformMatrix3fv(glGetUniformLocation(m_shaderProgram, name), 1, GL_FALSE, value);
	glUseProgram(GL_NONE);
	return true;
}

bool Material::SetUniform(const char* name, lm::mat4 value) {
	BindMaterial();
	if (glGetUniformLocation(m_shaderProgram, name) == -1)
		return false;
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, name), 1, GL_FALSE, value);
	glUseProgram(GL_NONE);
	return true;
}

bool Material::SetUniform(const char* name, lm::vec2 value) {
	BindMaterial();
	if (glGetUniformLocation(m_shaderProgram, name) == -1)
		return false;
	glUniform2fv(glGetUniformLocation(m_shaderProgram, name), 1, &value.x);
	glUseProgram(GL_NONE);
	return true;
}

bool Material::SetUniform(const char* name, lm::vec3 value) {
	BindMaterial();
	if (glGetUniformLocation(m_shaderProgram, name) == -1)
		return false;
	glUniform3fv(glGetUniformLocation(m_shaderProgram, name), 1, &value.x);
	glUseProgram(GL_NONE);
	return true;
}

bool Material::SetUniform(const char* name, lm::vec4 value) {
	BindMaterial();
	if (glGetUniformLocation(m_shaderProgram, name) == -1)
		return false;
	glUniform4fv(glGetUniformLocation(m_shaderProgram, name), 1, &value.x);
	glUseProgram(GL_NONE);
	return true;
}

bool Material::UniformBlockBinding(const char* name, const int index) {
	BindMaterial();
	if (glGetUniformBlockIndex(m_shaderProgram, name) == -1)
		return false;
	glUniformBlockBinding(m_shaderProgram, glGetUniformBlockIndex(m_shaderProgram, name), index);
	glUseProgram(GL_NONE);
	return true;
}

void Material::SetTexture(const TextureSlot slot, shared_ptr<Texture> texture) {
	if (texture == nullptr) {
		return;
	}
	m_textures[slot] = texture;

	SetUniform(("u_Has" + m_textureNames[slot] + "Tex").c_str(), true);
}

void Material::CopyTexturesTo(Material* dest) {
	for (pair<const TextureSlot, shared_ptr<Texture>>& tex : m_textures) {
		dest->SetTexture(tex.first, tex.second);
	}
}

void Material::SetAlphaMethod(const AlphaType method) {
	m_alphaMethod = method;
	
	m_shaders[FragmentShader]->SetMacro("transparent", method != AlphaNone);
}

shared_ptr<Shader> Material::GetShader(const ShaderType type) {
	if (m_shaders.find(type) == m_shaders.end()) {
		return nullptr;
	}
	return m_shaders[type];
}

void Material::Recompile() {
	glDeleteProgram(m_shaderProgram);
	m_shaderProgram = glCreateProgram();

	for (const auto& shader : m_shaders) {
		shader.second->Bind(m_shaderProgram);
	}

	glLinkProgram(m_shaderProgram);
	
	glValidateProgram(m_shaderProgram);

	int success;
	glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
		GlenderLog(LogTypeError, format("Material linking failed with message: {}", infoLog));
	}

	for (const auto& shader : m_shaders) {
		shader.second->Unbind(m_shaderProgram);
	}
	
	glUseProgram(m_shaderProgram);

	SetUniform("u_AlbedoTex", 0);
	SetUniform("u_NormalTex", 1);
	SetUniform("u_SpecularTex", 2);
	SetUniform("u_RoughnessTex", 3);
	SetUniform("u_MetalicTex", 4);
	SetUniform("u_AOTex", 5);

	SetUniform("u_HasAlbedoTex", false);
	SetUniform("u_HasNormalTex", false);
	SetUniform("u_HasSpecularTex", false);
	SetUniform("u_HasRoughnessTex", false);
	SetUniform("u_HasMetalicTex", false);
	SetUniform("u_HasAOTex", false);

	UniformBlockBinding("Metrices", 2);

	for (const pair<TextureSlot, shared_ptr<Texture>>& tex : m_textures) {
		SetUniform(("u_Has" + m_textureNames[tex.first] + "Tex").c_str(), true);
	}

	Unbind();
}


PbrMaterial::PbrMaterial(const vector<shared_ptr<Shader>>& shaders, const lm::vec4 albedo, 
	const float specular, const float roughness, const float metalic) :
		Material(MaterialType::MaterialTypePbr, shaders), m_albedo(albedo), m_specular(specular), m_roughness(roughness), m_metalic(metalic) {

	UpdateUniforms();
}

PbrMaterial::PbrMaterial(const lm::vec4 albedo, const float specular, const float roughness, const float metalic, 
									const vector<filesystem::path>& shaderPaths) :
		Material(MaterialType::MaterialTypePbr, shaderPaths), m_albedo(albedo), m_specular(specular), m_roughness(roughness), m_metalic(metalic) {

	UpdateUniforms();
}

void PbrMaterial::SetAlbedo(lm::vec4 value) {
	m_albedo = value;
	SetUniform("u_Albedo", value);
}

void PbrMaterial::SetSpecular(const float value) {
	m_specular = value;
	SetUniform("u_Specular", value);
}

void PbrMaterial::SetRoughness(const float value) {
	m_roughness = value;
	SetUniform("u_Roughness", value);
}

void PbrMaterial::SetMetalic(const float value) {
	m_metalic = value;
	SetUniform("u_Metalic", value);
}

lm::vec4 PbrMaterial::GetAlbedo() const {
	return m_albedo;
}

float PbrMaterial::GetSpecular() const {
	return m_specular;
}

float PbrMaterial::GetRoughness() const {
	return m_roughness;
}

float PbrMaterial::GetMetalic() const {
	return m_metalic;
}

void PbrMaterial::CopyAttributesFrom(PbrMaterial* from) {
	SetAlbedo(from->GetAlbedo());
	SetSpecular(from->GetSpecular());
	SetRoughness(from->GetRoughness());
	SetMetalic(from->GetMetalic());
}

void PbrMaterial::UpdateUniforms() {
	SetUniform("u_Albedo", m_albedo);
	SetUniform("u_Specular", m_specular);
	SetUniform("u_Roughness", m_roughness);
	SetUniform("u_Metalic", m_metalic);
}
