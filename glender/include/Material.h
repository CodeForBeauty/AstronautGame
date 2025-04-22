#pragma once

#include "Shader.h"
#include "Texture.h"

#include <lm/lm.h>

#include <vector>
#include <unordered_map>
#include <filesystem>
#include <memory>


namespace glender {
	enum TextureSlot {
		TextureAlbedo,
		TextureNormal,
		TextureSpecular,
		TextureRoughness,
		TextureMetalic,
		TextureAO
	};

	enum MaterialType {
		MaterialTypeNone,
		MaterialTypePbr,
		MaterialTypeNormal,
		MaterialTypeCompute,
	};

	enum AlphaType {
		AlphaNone,
		AlphaCut,
		AlphaBlend,
	};

	class Material {
	public:
		Material(const MaterialType type, const std::vector<std::shared_ptr<Shader>>& shaders);
		Material(const MaterialType type, const std::vector<std::filesystem::path>& shaderPaths = { "shaders/Default.vert", "shaders/Default.frag", "shaders/Default.geom"});
		Material() : m_shaderProgram(NULL), Type(MaterialTypeNone), m_alphaMethod(AlphaNone) {};
		~Material();

		void Bind();
		void Unbind();

		void LoadShaders(const std::vector<std::shared_ptr<Shader>>& shaders);
		void LoadShaders(const std::vector<std::filesystem::path>& shaderPaths);

		bool SetUniform(const char* name, const int value);
		bool SetUniform(const char* name, const float value);
		bool SetUniform(const char* name, const bool value);
		bool SetUniform(const char* name, lm::mat2 value);
		bool SetUniform(const char* name, lm::mat3 value);
		bool SetUniform(const char* name, lm::mat4 value);
		bool SetUniform(const char* name, lm::vec2 value);
		bool SetUniform(const char* name, lm::vec3 value);
		bool SetUniform(const char* name, lm::vec4 value);
		bool UniformBlockBinding(const char* name, const int index);

		void SetTexture(const TextureSlot slot, std::shared_ptr<Texture> texture);

		void CopyTexturesTo(Material* dest);

		MaterialType Type;

		void SetAlphaMethod(const AlphaType method);

		std::shared_ptr<Shader> GetShader(const ShaderType type);

		void Recompile();

	protected:
		std::unordered_map<TextureSlot, std::shared_ptr<Texture>> m_textures;
		std::unordered_map<ShaderType, std::shared_ptr<Shader>> m_shaders;

		AlphaType m_alphaMethod;

		inline static std::unordered_map<TextureSlot, std::string> m_textureNames{
			{TextureAlbedo, "Albedo"},
			{TextureNormal, "Normal"},
			{TextureSpecular, "Specular"},
			{TextureRoughness, "Roughness"},
			{TextureMetalic, "Metalic"},
			{TextureAO, "AO"}
		};
	private:
		unsigned int m_shaderProgram;
	};

	class PbrMaterial : public Material {
	public:
		PbrMaterial(const std::vector<std::shared_ptr<Shader>>& shaders, 
			const lm::vec4 albedo = {1.0f, 1.0f, 1.0f, 1.0f}, const float specular = 0.5f, 
			const float roughness = 0.5f, const float metalic = 0.0f);
		PbrMaterial(const lm::vec4 albedo = { 1.0f, 1.0f, 1.0f, 1.0f }, const float specular = 0.5f, 
			const float roughness = 0.5f, const float metalic = 0.0f,
			const std::vector<std::filesystem::path>& shaderPaths = { "shaders/Default.vert", "shaders/Default.frag", "shaders/Default.geom" });

		void SetAlbedo(lm::vec4 value);
		void SetSpecular(const float value);
		void SetRoughness(const float value);
		void SetMetalic(const float value);

		lm::vec4 GetAlbedo() const;
		float GetSpecular() const;
		float GetRoughness() const;
		float GetMetalic() const;

		void CopyAttributesFrom(PbrMaterial* from);

	protected:
		lm::vec4 m_albedo;
		float m_specular;
		float m_roughness;
		float m_metalic;

		void UpdateUniforms();
	private:

	};
}