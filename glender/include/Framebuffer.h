#pragma once

#include "Texture.h"
#include "Types.h"

#include <lm/lm.h>
#include <memory>
#include <vector>

namespace glender {
	enum FramebufferBindType {
		FramebufferReadWrite = GL_FRAMEBUFFER,
		FramebufferRead = GL_READ_FRAMEBUFFER,
		FramebufferWrite = GL_DRAW_FRAMEBUFFER
	};

	class Framebuffer {
	public:
		Framebuffer(const int2& resolution);
		~Framebuffer();

		void Bind(const FramebufferBindType type = FramebufferReadWrite) const;
		void Unbind() const;
		void Clear(const lm::vec4& color = {0, 0, 0, 1});

		std::shared_ptr<Texture> AddTexture(const TextureDataType dataType, const TextureType type = TextureType2D, const TextureUse use = TextureUseColor);
		void AddTexture(std::shared_ptr<Texture> texture);
		void AddTexture(Texture* texture);

		void ChangeMipLevel(std::shared_ptr<Texture> texture, const int mip);
		void ChangeMipLevel(Texture* texture, const int mip);

		void BlitFrom(Framebuffer& from);

		void BindTextures(const int offset = 0) const;
		void UnbindTextures();

		int2 GetSize() const;
		void Resize(const int2& resolution);

		template <typename OutType>
		OutType* ReadPixel(const unsigned int x, const unsigned int y, const int index);

	protected:
		unsigned int m_fbo;

		int2 m_resolution;

		std::vector<std::shared_ptr<Texture>> m_textures;
		std::vector<Texture*> m_unmanaged;

		void UpdateAttachments();
	private:
		std::vector<unsigned int> m_attachments;
	};

	template<typename OutType>
	inline OutType* Framebuffer::ReadPixel(const unsigned int x, const unsigned int y, const int index) {
		Bind();

		glReadBuffer(GL_COLOR_ATTACHMENT0 + index);
		OutType* pixel = new OutType[4] { static_cast<OutType>(0), static_cast<OutType>(0), static_cast<OutType>(0), static_cast<OutType>(0) };
		
		m_textures[index]->ReadPixel(x, y, pixel);

		Unbind();
		return pixel;
	}
}