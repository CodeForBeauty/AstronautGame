#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Types.h"
#include "Entity.h"
#include "Material.h"

#include <vector>
#include <memory>


namespace glender {
	struct Vertex {
		lm::vec3 pos;
		lm::vec3 normal;
		lm::vec3 tangent;
	};

	struct VertexStatic {
		lm::vec2 uv;
		lm::vec3 color;
	};

	enum AttribType {
		AttribuDouble = GL_DOUBLE,
		AttribFloat = GL_FLOAT,
		//AttribHFloat = GL_HALF_FLOAT,
		AttribByte = GL_BYTE,
		AttribUByte = GL_UNSIGNED_BYTE,
		AttribInt = GL_INT,
		AttribUInt = GL_UNSIGNED_INT,
	};

	class Geometry {
	public:
		Geometry(const std::vector<Vertex>& verticesDyn, const std::vector<VertexStatic>& vericesSt, const std::vector<unsigned int> indices);
		Geometry();
		~Geometry();
		
		void Bind() const;
		void Unbind() const;

		void SetGeometry(const std::vector<Vertex>& verticesDyn, const std::vector<VertexStatic>& vericesSt, const std::vector<unsigned int>& indices);

		template <class Type>
		void AddAttribute(std::string name, std::vector<Type>& data, unsigned int elements, AttribType type);

		template <class Type>
		void UpdateAttribute(std::string name, std::vector<Type>& data);

		void Draw();

	protected:
		std::vector<Vertex> VerticesDyn;
		std::vector<VertexStatic> VerticesSt;
		std::vector<unsigned int> Indices;

	private:
		unsigned int m_vbo;
		unsigned int m_vboSt;
		unsigned int m_vao;
		unsigned int m_ebo;

		std::unordered_map<std::string, unsigned int> m_attributes;
	};

	template<class Type>
	inline void glender::Geometry::AddAttribute(std::string name, std::vector<Type>& data, unsigned int elements, AttribType type) {
		unsigned int tmpVbo = 0;

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

		glGenBuffers(1, &tmpVbo);
		glBindBuffer(GL_ARRAY_BUFFER, tmpVbo);

		m_attributes[name] = tmpVbo;

		glBufferData(GL_ARRAY_BUFFER, sizeof(Type) * data.size(), data.data(), GL_STATIC_DRAW);

		if (type == AttribUInt || type == AttribInt) {
			glVertexAttribIPointer(4 + m_attributes.size(), elements, type, sizeof(Type), (void*)0);
		}
		else {
			glVertexAttribPointer(4 + m_attributes.size(), elements, type, GL_FALSE, sizeof(Type), (void*)0);
		}
		glEnableVertexAttribArray(4 + m_attributes.size());
	}

	template<class Type>
	inline void glender::Geometry::UpdateAttribute(std::string name, std::vector<Type>& data) {
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

		glBindBuffer(GL_ARRAY_BUFFER, m_attributes[name]);

		glBufferData(GL_ARRAY_BUFFER, sizeof(Type) * data.size(), data.data(), GL_STATIC_DRAW);
	}

	class Mesh : public Entity, public Geometry {
	public:
		Mesh(std::shared_ptr<Material> material);

		virtual void RenderMesh();

		void GetVertices(std::vector<Vertex>& verticesDyn, std::vector<VertexStatic>& verticesSt, std::vector<unsigned int>& indices, int indexOffset = 0);

		int SceneIndex;

	protected:
		std::shared_ptr<Material> m_material;

	private:

	};

	class SolidMesh : public Mesh {
	public:
		SolidMesh(std::shared_ptr<Material> material);
	};

	class OverlayMesh : public Mesh {
	public:
		OverlayMesh(std::shared_ptr<Material> material);
	};
}