#include "Shader.h"

#include "Logging.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>

using namespace glender;
using namespace std;


Shader::Shader(ShaderType type) {
	switch (type) {
	case VertexShader:
		m_shaderText = "#version 330 core\n"
						"layout (location = 0) in vec3 aPos;\n"
						"void main()\n"
						"{\n"
						"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
						"}\0";
		break;
	case FragmentShader:
		m_shaderText = "#version 330 core\n"
						"out vec4 FragColor;\n"
						"void main() {\n"
						"	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
						"}\0";
		break;
	case GeometryShader:
		break;
	case ComputeShader:
		break;
	default:
		break;
	}

	LoadShader(type);
}

Shader::Shader(const ShaderType type, const string& shaderText) {
	m_shaderText = shaderText;
	LoadShader(type);
}

Shader::Shader(const filesystem::path& filepath) {
	LoadFromFile(filepath);
}

Shader::~Shader() {
	glDeleteShader(m_shaderID);
}

void Shader::CompileShader() {
	glCompileShader(m_shaderID);
	

	int success;
	glGetShaderiv(m_shaderID, GL_COMPILE_STATUS, &success);

	if (!success) {
		int maxLength = 0;
		glGetShaderiv(m_shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		string infoLog;
		infoLog.resize(maxLength);
		glGetShaderInfoLog(m_shaderID, maxLength, &maxLength, &infoLog[0]);
		GlenderLog(LogTypeError, format("{} Shader compilation failed, with message: {}.\nSource path: {}", 
			m_shaderTypeNames[m_type], infoLog, m_shaderPath.string()));
	}
}

void Shader::Unbind(unsigned int shaderProgram) const {
	glDetachShader(shaderProgram, m_shaderID);
}

void Shader::Bind(unsigned int shaderProgram) const {
	glAttachShader(shaderProgram, m_shaderID);
}

void Shader::UpdateText(const string& text) {
	m_shaderText = text;
	LoadShader(m_type);
}

void Shader::UpdateText(const filesystem::path& file) {
	UpdateFromFile(file);
}

void Shader::SetMacro(const string& macro, const bool value) {
	m_bools[macro] = value;

	Format();
}

void Shader::SetMacro(const string& macro, const string& value) {
	m_values[macro] = value;

	Format();
}

ShaderType Shader::GetType() const {
	return m_type;
}

void Shader::Format() {
	m_formatedText = Format(m_shaderText);
}

string Shader::Format(const string& text) {
	string result = "";
	istringstream s(text);

	for (string line; getline(s, line); ) {
		if (!line.starts_with("//#")) {
			while (line.contains("//{")) {
				int first = line.find_first_of("//{");
				int last = line.find_first_of("}", first);
				string macro = line.substr(first + 3, last - (first + 3));
				if (m_values[macro].empty()) {
					GlenderLog(LogTypeWarning, format("Macro: {}, was not initialized in a shader: {}.", macro, m_shaderPath.string()));
					line.replace(first, last + 2 - first, "");
				}
				else {
					first = line.find_last_of(" ", first);
					line.replace(first, last + 1 - first, m_values[macro]);
				}
			}
			result += line + "\n";
			continue;
		}

		string directive = line.substr(0, line.find_first_of(" ", 0));

		if (directive == "//#include") {
			filesystem::path shaderDir = m_shaderPath;
			shaderDir.remove_filename();
			filesystem::path filepath = shaderDir / line.substr(line.find_first_of(" ", 0) + 1, line.size());
			if (!filesystem::exists(filepath)) {
				GlenderLog(LogTypeWarning, format("Failed to include file in a shader. Filepath: {}", filepath.string()));
				continue;
			}
			ifstream file(filepath);
			string content;
			string line;

			while (getline(file, line)) {
				content += line + "\n";
			}

			string formated = Format(content);

			result += formated;
			cout << formated;

			file.close();
		}
		else if (directive == "//#if") {
			string content;

			string macro = line.substr(6, line.size());

			getline(s, line);
			while (!line.starts_with("//#endif")) {
				content += line + "\n";
				getline(s, line);
			}

			if (m_bools[macro]) {
				content = Format(content);

				result += content;
			}

		}
	}
	return result;
}

void Shader::LoadShader(const ShaderType type) {
	m_type = type;

	m_shaderID = glCreateShader(type);

	Format();
	
	const char* shaderText = m_formatedText.c_str();
	glShaderSource(m_shaderID, 1, &shaderText, NULL);

	CompileShader();
}

void Shader::LoadFromFile(const filesystem::path& filepath) {
	if (!TextFromFile(filepath)) {
		return;
	}

	ShaderType type = VertexShader;
	string extension = filepath.extension().string();
	if (extension == ".vert") {
		type = VertexShader;
	}
	else if (extension == ".frag") {
		type = FragmentShader;
	}
	else if (extension == ".geom") {
		type = GeometryShader;
	}
	else if (extension == ".comp") {
		type = ComputeShader;
	}
	else {
		GlenderLog(LogTypeError, format("Unknown shader format: {}.", extension));
		return;
	}

	LoadShader(type);
}

void Shader::UpdateFromFile(const filesystem::path& filepath) {
	ShaderType type = VertexShader;
	string extension = filepath.extension().string();
	if (extension == ".vert") {
		type = VertexShader;
	}
	else if (extension == ".frag") {
		type = FragmentShader;
	}
	else if (extension == ".geom") {
		type = GeometryShader;
	}
	else if (extension == ".comp") {
		type = ComputeShader;
	}
	else {
		GlenderLog(LogTypeError, format("Unknown shader format: {}.", extension));
		return;
	}

	if (type != m_type) {
		GlenderLog(LogTypeError, format("Trying to load {} shader as a {} shader. Shader path: {}",
			m_shaderTypeNames[type], m_shaderTypeNames[m_type], filepath.string()));
		return;
	}

	if (!TextFromFile(filepath)) {
		return;
	}

	Format();

	glDeleteShader(m_shaderID);
	m_shaderID = glCreateShader(m_type);

	const char* shaderText = m_formatedText.c_str();
	glShaderSource(m_shaderID, 1, &shaderText, NULL);

	CompileShader();
}

bool Shader::TextFromFile(const filesystem::path& filepath) {
	if (!filesystem::exists(filepath)) {
		GlenderLog(LogTypeError, format("Shader file does not exist: {}", filepath.string()));
		return false;
	}

	m_shaderPath = filepath;

	ifstream file(filepath);
	string content;
	string line;

	while (getline(file, line)) {
		content += line + "\n";
	}

	file.close();
	m_shaderText = content;

	return true;
}
