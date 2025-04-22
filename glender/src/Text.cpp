#include "Text.h"

using namespace glender;
using namespace std;


TextMesh::TextMesh(shared_ptr<Material> material, shared_ptr<FontMap> font, const string& text, float scale)
		: Mesh(material), m_textScale(scale), m_text(text), m_font(font) {
	RegenerateMesh();

	EntityTransform.SetName("TextMesh");
}

float TextMesh::GetTextScale() const {
	return m_textScale;
}

void TextMesh::SetTextScale(float newScale) {
	m_textScale = newScale;

	RegenerateMesh();
}

void TextMesh::ChangeText(const string& newText) {
	m_text = newText;

	RegenerateMesh();
}

void TextMesh::RenderMesh() {
	m_material->SetUniform("u_Fontmap", 6);
	lm::vec2 fontRes = { (float)m_font->GetSize().x, (float)m_font->GetSize().y };
	m_material->SetUniform("u_FontmapSize", fontRes);
	m_material->SetUniform("u_FontWidth", 0.15f);
	m_font->Bind(6);
	Mesh::RenderMesh();
	m_font->Unbind();
}

void TextMesh::RegenerateMesh() {
	lm::vec2 pos = { 0, 0 };

	vector<Vertex> verticesDyn;
	vector<VertexStatic> verticesSt;
	vector<unsigned int> indices;

	for (char ch : m_text) {
		FontGlyphData glyph = m_font->GetGlyphData(ch);

		float diffX = glyph.UVMax.x - glyph.UVMin.x;
		float diffY = glyph.UVMax.y - glyph.UVMin.y;

		diffX *= m_textScale;
		diffY *= m_textScale;
		
		unsigned int sizeDyn = verticesDyn.size();
		indices.insert(indices.end(), {sizeDyn + 2, sizeDyn + 1, sizeDyn, 
			sizeDyn, sizeDyn + 3, sizeDyn + 2});

		float bearing = glyph.Bearing * m_textScale;

		verticesDyn.push_back({ { pos.x, pos.y - bearing, 0.0f }, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} });
		verticesDyn.push_back({ { pos.x, pos.y + diffY - bearing, 0.0f }, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} });
		verticesDyn.push_back({ { pos.x + diffX, pos.y + diffY - bearing, 0.0f }, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} });
		verticesDyn.push_back({ { pos.x + diffX, pos.y - bearing, 0.0f }, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} });

		verticesSt.push_back({ glyph.UVMin });
		verticesSt.push_back({ {glyph.UVMin.x, glyph.UVMax.y} });
		verticesSt.push_back({ glyph.UVMax });
		verticesSt.push_back({ {glyph.UVMax.x, glyph.UVMin.y} });

		pos.x += diffX;
	}

	SetGeometry(verticesDyn, verticesSt, indices);
}
