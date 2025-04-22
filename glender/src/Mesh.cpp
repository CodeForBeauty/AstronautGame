#include "Mesh.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

using namespace glender;
using namespace std;


Geometry::Geometry(const vector<Vertex>& verticesDyn, const vector<VertexStatic>& vericesSt, const vector<unsigned int> indices)
		: m_vbo(GL_NONE), m_vao(GL_NONE), m_ebo(GL_NONE), m_vboSt(GL_NONE) {
	SetGeometry(verticesDyn, vericesSt, indices);
}

Geometry::Geometry() : m_vbo(GL_NONE), m_vao(GL_NONE), m_ebo(GL_NONE), m_vboSt(GL_NONE) {
	SetGeometry(vector<Vertex>(), vector<VertexStatic>(), vector<unsigned int>());
}

Geometry::~Geometry() {
	glDeleteBuffers(1, &m_vbo);
	glDeleteBuffers(1, &m_vboSt);

	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_ebo);
}

void Geometry::Bind() const {
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
}

void Geometry::Unbind() const {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Geometry::SetGeometry(const vector<Vertex>& verticesDyn, const vector<VertexStatic>& vericesSt,
		const vector<unsigned int>& indices) {
	VerticesDyn = verticesDyn;
	VerticesSt = vericesSt;
	Indices = indices;

	if (!m_vbo) {
		glGenBuffers(1, &m_vbo);
		glGenVertexArrays(1, &m_vao);
		glGenBuffers(1, &m_ebo);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
		
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * VerticesDyn.size(), VerticesDyn.data(), GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), Indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
		glEnableVertexAttribArray(2);
		

		glGenBuffers(1, &m_vboSt);
		glBindBuffer(GL_ARRAY_BUFFER, m_vboSt);

		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexStatic) * VerticesSt.size(), VerticesSt.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexStatic), (void*)offsetof(VertexStatic, uv));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStatic), (void*)offsetof(VertexStatic, color));
		glEnableVertexAttribArray(4);
	}
	else {
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * VerticesDyn.size(), VerticesDyn.data(), GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), Indices.data(), GL_STATIC_DRAW);


		glBindBuffer(GL_ARRAY_BUFFER, m_vboSt);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexStatic) * VerticesSt.size(), VerticesSt.data(), GL_STATIC_DRAW);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Geometry::Draw() {
	Bind();
	glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);
	Unbind();
}


Mesh::Mesh(shared_ptr<Material> material) : Entity(), Geometry(), m_material(material), SceneIndex(0) {
	EntityTransform.SetName("Mesh");
}

void Mesh::RenderMesh() {
	m_material->SetUniform("u_ObjectIndex", SceneIndex);
	m_material->SetUniform("u_Transform", EntityTransform.WorldMatrix);
	m_material->Bind();
	
	Draw();

	m_material->Unbind();
}

void Mesh::GetVertices(vector<Vertex>& verticesDyn, vector<VertexStatic>& verticesSt, vector<unsigned int>& indices, int indexOffset) {
	for (Vertex vertex : VerticesDyn) {
		lm::vec4 pos = vertex.pos;
		pos.w = 1.0f;
		pos = EntityTransform.WorldMatrix * pos;
		vertex.pos = { pos.x, pos.y, pos.z };
		verticesDyn.push_back(vertex);
	}
	for (unsigned int index : Indices) {
		indices.push_back(index + indexOffset);
	}
	copy(VerticesSt.begin(), VerticesSt.end(), back_inserter(verticesSt));
}

SolidMesh::SolidMesh(shared_ptr<Material> material) : Mesh(material) {
	SetGeometry(
		{
		{{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
		{{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
		{{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}}
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
		});
	EntityTransform.SetName("SolidMesh");
}

OverlayMesh::OverlayMesh(shared_ptr<Material> material) : Mesh(material) {
	SetGeometry(
		{
		{{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
		{{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
		{{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}}
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
		});
	EntityTransform.SetName("OverlayMesh");
}