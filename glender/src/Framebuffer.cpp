#include "Framebuffer.h"

#include "Logging.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

using namespace glender;
using namespace std;


Framebuffer::Framebuffer(const int2& resolution) : m_resolution(resolution), m_unmanaged() {
	if (resolution.x <= 0 || resolution.y <= 0) {
		GlenderLog(LogTypeError, format("Framebuffer resolution is incorrect. Values width: {}, height: {} "
			"Values should satisfy: width > 0 && height > 0.", resolution.x, resolution.y));
	}
	glGenFramebuffers(1, &m_fbo);
}

Framebuffer::~Framebuffer() {
	glDeleteFramebuffers(1, &m_fbo);
}

void Framebuffer::Bind(const FramebufferBindType type) const {
	glBindFramebuffer(type, m_fbo);
	
	glViewport(0, 0, m_resolution.x, m_resolution.y);
}

void Framebuffer::Unbind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

void Framebuffer::Clear(const lm::vec4& color) {
	glClearColor(color.x, color.y, color.z, color.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

shared_ptr<Texture> Framebuffer::AddTexture(const TextureDataType dataType, const TextureType type, const TextureUse use) {
	Bind();
	
	shared_ptr<Texture> tex = m_textures.emplace_back(new Texture(m_resolution, type, dataType, use));
	tex->BindToFramebuffer(m_textures.size() + m_unmanaged.size() - 1);

	switch (use) {
	case TextureUse::TextureUseDepth:
		break;
	default:
		m_attachments.push_back(GL_COLOR_ATTACHMENT0 + m_textures.size() + m_unmanaged.size() - 1);
		break;
	}

	UpdateAttachments();
	return tex;
}

void Framebuffer::AddTexture(shared_ptr<Texture> texture) {
	Bind();

	m_textures.push_back(texture);
	texture->BindToFramebuffer(m_textures.size() + m_unmanaged.size() - 1);

	switch (texture->GetUse()) {
	case TextureUse::TextureUseDepth:
		break;
	default:
		m_attachments.push_back(GL_COLOR_ATTACHMENT0 + m_textures.size() + m_unmanaged.size() - 1);
		break;
	}

	UpdateAttachments();
}

void Framebuffer::AddTexture(Texture* texture) {
	Bind();

	m_unmanaged.push_back(texture);
	texture->BindToFramebuffer(m_textures.size() + m_unmanaged.size() - 1);

	switch (texture->GetUse()) {
	case TextureUse::TextureUseDepth:
		break;
	default:
		m_attachments.push_back(GL_COLOR_ATTACHMENT0 + m_textures.size() + m_unmanaged.size() - 1);
		break;
	}

	UpdateAttachments();
}

void Framebuffer::ChangeMipLevel(shared_ptr<Texture> texture, const int mip) {
	Bind();

	vector<shared_ptr<Texture>>::iterator pos = find(m_textures.begin(), m_textures.end(), texture);
	if (pos == m_textures.end()) {
		GlenderLog(LogTypeWarning, format("Failed to find texture: {} to change mip level.", texture->GetTexturePath().string()));
		return;
	}
	int index = distance(pos, m_textures.begin());
	pos[0]->BindToFramebuffer(index, mip);
}

void Framebuffer::ChangeMipLevel(Texture* texture, const int mip) {
	Bind();

	vector<Texture*>::iterator pos = find(m_unmanaged.begin(), m_unmanaged.end(), texture);
	if (pos == m_unmanaged.end()) {
		GlenderLog(LogTypeWarning, format("Failed to find texture: {} to change mip level.", texture->GetTexturePath().string()));
		return;
	}
	int index = distance(pos, m_unmanaged.begin());
	pos[0]->BindToFramebuffer(index, mip);
}

void Framebuffer::BlitFrom(Framebuffer& from) {
	from.Bind(FramebufferRead);
	Bind(FramebufferWrite);
	int2 fromSize = from.GetSize();
	if (m_attachments.size() > 0) {
		glBlitFramebuffer(0, 0, fromSize.x, fromSize.y, 0, 0, m_resolution.x, m_resolution.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
	if (m_attachments.size() < (m_textures.size() + m_unmanaged.size())) {
		glBlitFramebuffer(0, 0, fromSize.x, fromSize.y, 0, 0, m_resolution.x, m_resolution.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}
}

void Framebuffer::BindTextures(int offset) const {
	for (int i = 0; i < m_textures.size(); i++) {
		m_textures[i]->Bind(i + offset);
	}
}

int2 Framebuffer::GetSize() const {
	return m_resolution;
}

void Framebuffer::Resize(const int2& resolution) {
	if (resolution.x <= 0 || resolution.y <= 0) {
		GlenderLog(LogTypeError, format("Framebuffer resolution is incorrect. Values width: {}, height: {} "
			"Values should satisfy: width > 0 && height > 0.", resolution.x, resolution.y));
	}
	m_resolution = resolution;

	for (int i = 0; i < m_textures.size(); i++) {
		m_textures[i]->ResizeTexture(TextureResizeClear, resolution);
	}
}

void Framebuffer::UpdateAttachments() {
	Bind();

	if (m_attachments.size() > 0) {
		glDrawBuffers(m_attachments.size(), m_attachments.data());
	}

	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	switch (fboStatus) {
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		GlenderLog(LogTypeError, "Framebuffer status: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		GlenderLog(LogTypeError, "Framebuffer status: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED:
		GlenderLog(LogTypeError, "Framebuffer status: GL_FRAMEBUFFER_UNSUPPORTED");
		break;
	}

	Unbind();
}
