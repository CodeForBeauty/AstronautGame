#pragma once
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"

#include <string>
#include <memory>


namespace glender {
	class TextMesh : public Mesh {
	public:
		TextMesh(std::shared_ptr<Material> material, std::shared_ptr<FontMap> font, const std::string& text, const float scale = 5);
		
		float GetTextScale() const;
		void SetTextScale(const float newScale);

		void ChangeText(const std::string& newText);

		virtual void RenderMesh() override;

	protected:
		float m_textScale;
		std::string m_text;

		std::shared_ptr<FontMap> m_font;

		void RegenerateMesh();

	private:

	};
}