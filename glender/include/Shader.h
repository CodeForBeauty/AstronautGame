#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <string>
#include <filesystem>
#include <unordered_map>

namespace glender {
	enum ShaderType {
		VertexShader = GL_VERTEX_SHADER,
		FragmentShader = GL_FRAGMENT_SHADER,
		GeometryShader = GL_GEOMETRY_SHADER,
		ComputeShader = GL_COMPUTE_SHADER
	};

	class Shader {
	public:
		Shader(const ShaderType type);
		Shader(const ShaderType type, const std::string& shaderText);
		Shader(const std::filesystem::path& filepath);
		~Shader();

		void CompileShader();

		void Unbind(unsigned int shaderProgram) const;
		void Bind(unsigned int shaderProgram) const;

		void UpdateText(const std::string& text);
		void UpdateText(const std::filesystem::path& file);

		void SetMacro(const std::string& macro, const bool value);
		void SetMacro(const std::string& macro, const std::string& value);

		ShaderType GetType() const;
		

	protected:
		void Format();
		std::string Format(const std::string& text);

		inline static std::unordered_map<ShaderType, std::string> m_shaderTypeNames{
			{VertexShader, "Vertex"},
			{FragmentShader, "Fragment"},
			{GeometryShader, "Geometry"},
			{ComputeShader, "Compute"},
		};

	private:
		ShaderType m_type;
		unsigned int m_shaderID;

		std::string m_shaderText;
		std::string m_formatedText;
		std::filesystem::path m_shaderPath;

		std::unordered_map<std::string, bool> m_bools;
		std::unordered_map<std::string, std::string> m_values;

		void LoadShader(const ShaderType type);
		void LoadFromFile(const std::filesystem::path& filepath);

		void UpdateFromFile(const std::filesystem::path& filepath);
		bool TextFromFile(const std::filesystem::path& filepath);
	};
}