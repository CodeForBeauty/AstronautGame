#include "Texture.h"
#include "Framebuffer.h"
#include "Mesh.h"

#include "Logging.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H 

using namespace glender;
using namespace std;


Texture::Texture(const int2 resolution, const TextureType type, const TextureDataType dataType, 
		const TextureUse use, const TextureBitSize bitSize) :
		m_resolution(resolution), m_use(use), m_type(type), m_dataType(dataType), m_bitSize(bitSize),
		m_wrapType(TextureWrapClamp), m_minFilter(TextureMinFilterLinear), m_magFilter(TextureMagFilterLinear), m_channels(4) {

	if (use == TextureUseGrayscale) {
		m_channels = 1;
	}

	glCreateTexture();
	SetTextureData();
}

Texture::~Texture() {
	glDeleteTextures(1, &m_textureID);
}

void Texture::Bind(unsigned int slot) {
	if (m_textureID == GL_NONE) {
		return;
	}
	m_lastSlot = slot;
	glActiveTexture(GL_TEXTURE0 + slot);
	
	glBindTexture(m_type, m_textureID);

}

void Texture::Unbind() {
	glActiveTexture(GL_TEXTURE0 + m_lastSlot);
	glBindTexture(m_type, 0);
}

void Texture::BindToFramebuffer(const int index, const int mip) {
	Bind(0);
	

	unsigned int use1 = GL_COLOR_ATTACHMENT0;
	switch (m_use) {
	case TextureUseColor:
		use1 = GL_COLOR_ATTACHMENT0 + index;
		break;
	case TextureUseGrayscale:
		use1 = GL_COLOR_ATTACHMENT0 + index;
		break;
	case TextureUseDepth:
		use1 = GL_DEPTH_ATTACHMENT;
		break;
	}

	if (m_type == TextureTypeCubemap) {
		glFramebufferTexture(GL_FRAMEBUFFER, use1, m_textureID, mip);
	}
	else {
		glFramebufferTexture2D(GL_FRAMEBUFFER, use1, GL_TEXTURE_2D, m_textureID, mip);
	}

	Unbind();
}

void Texture::ResizeTexture(const TextureResizeType resizeType, const int2 newSize) {
	switch (resizeType) {
	case TextureResizeClear:
		m_resolution = newSize;
		SetTextureData();
		break;
	case TextureResizeAdd:
		break;
	case TextureResizeSample:
		break;
	default:
		GlenderLog(LogTypeError, "Unsupported texture resize type.");
		break;
	}
}

void Texture::SetData(const void* data, const int2 newSize) {
	Bind(0);
	m_resolution = newSize;
	unsigned int internal = GetInternalFormat();
	unsigned int format = GetFormat();

	switch (m_type) {
	case TextureType2D:
		glTexImage2D(GL_TEXTURE_2D, 0, internal, newSize.x, newSize.y, 0, format, m_dataType, data);
		break;
	case TextureTypeCubemap:
		for (int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal, newSize.x, newSize.y, 0, format, m_dataType,
				data);
		}
		break;
	}
	Unbind();
}

int2 Texture::GetSize() const {
	return m_resolution;
}

TextureUse Texture::GetUse() const {
	return m_use;
}

void Texture::SetTextureWrap(const TextureWrapType type) {
	Bind(0);
	m_wrapType = type;
	glTexParameteri(m_type, GL_TEXTURE_WRAP_S, type);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_T, type);
	glTexParameteri(m_type, GL_TEXTURE_WRAP_R, type);
	Unbind();
}

void Texture::SetTextureMinFilter(const TextureMinFilterType type) {
	Bind(0);
	m_minFilter = type;
	glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, type);
	Unbind();
}

void Texture::SetTextureMagFilter(const TextureMagFilterType type) {
	Bind(0);
	m_magFilter = type;
	glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, type);
	Unbind();
}

void Texture::GenerateMipmaps() {
	Bind(0);
	glGenerateMipmap(m_type);
	Unbind();
}

void Texture::ReadPixel(unsigned int x, unsigned int y, void* data) {
	if (m_use == TextureUseDepth) {
		glReadBuffer(GL_DEPTH_ATTACHMENT);
	}
	unsigned int format = GetFormat();
	glReadPixels(x, y, 1, 1, format, m_dataType, data);
}

unsigned int Texture::GetInternalFormat() const {
	unsigned int format = m_use;

	if (m_use == TextureUseColor) {
		switch (m_dataType) {
		case TextureDataUInt:
			switch (m_bitSize) {
			case TextureBit4:
				format = GL_RGBA8UI;
				break;
			case TextureBit8:
				format = GL_RGBA8UI;
				break;
			case TextureBit16:
				format = GL_RGBA16UI;
				break;
			case TextureBit32:
				format = GL_RGBA32UI;
				break;
			default:
				format = GL_RGBA16UI;
				break;
			}
			break;
		case TextureDataFloat:
			switch (m_bitSize) {
			case TextureBit4:
				format = GL_RGBA16F;
				break;
			case TextureBit8:
				format = GL_RGBA16F;
				break;
			case TextureBit16:
				format = GL_RGBA16F;
				break;
			case TextureBit32:
				format = GL_RGBA32F;
				break;
			default:
				format = GL_RGBA16F;
				break;
			}
			break;
		case TextureDataUByte:
			switch (m_bitSize) {
			case TextureBit4:
				format = GL_RGBA4;
				break;
			case TextureBit8:
				format = GL_RGBA8;
				break;
			case TextureBit16:
				format = GL_RGBA16;
				break;
			case TextureBit32:
				format = GL_RGBA;
				break;
			default:
				format = GL_RGBA16;
				break;
			}
			break;
		case TextureDataInt:
			switch (m_bitSize) {
			case TextureBit4:
				format = GL_RGBA8I;
				break;
			case TextureBit8:
				format = GL_RGBA8I;
				break;
			case TextureBit16:
				format = GL_RGBA16I;
				break;
			case TextureBit32:
				format = GL_RGBA32I;
				break;
			default:
				format = GL_RGBA16I;
				break;
			}
			break;
		default:
			format = GL_RGBA;
			break;
		}
	}
	else if (m_use == TextureUseDepth) {
		switch (m_dataType) {
		case TextureDataFloat:
			format = GL_DEPTH_COMPONENT32F;
			break;
		case TextureDataUByte:
			switch (m_bitSize) {
			case TextureBit4:
				format = GL_DEPTH_COMPONENT16;
				break;
			case TextureBit8:
				format = GL_DEPTH_COMPONENT16;
				break;
			case TextureBit16:
				format = GL_DEPTH_COMPONENT16;
				break;
			case TextureBit32:
				format = GL_DEPTH_COMPONENT32;
				break;
			default:
				format = GL_DEPTH_COMPONENT16;
				break;
			}
			break;
		default:
			format = GL_DEPTH_COMPONENT;
			break;
		}
	}
	else if (m_use == TextureUseGrayscale) {
		switch (m_dataType) {
		case TextureDataUInt:
			switch (m_bitSize) {
			case TextureBit4:
				format = GL_R8UI;
				break;
			case TextureBit8:
				format = GL_R8UI;
				break;
			case TextureBit16:
				format = GL_R16UI;
				break;
			case TextureBit32:
				format = GL_R32UI;
				break;
			default:
				format = GL_R16UI;
				break;
			}
			break;
		case TextureDataFloat:
			switch (m_bitSize) {
			case TextureBit4:
				format = GL_R16F;
				break;
			case TextureBit8:
				format = GL_R16F;
				break;
			case TextureBit16:
				format = GL_R16F;
				break;
			case TextureBit32:
				format = GL_R32F;
				break;
			default:
				format = GL_R16F;
				break;
			}
			break;
		case TextureDataUByte:
			switch (m_bitSize) {
			case TextureBit4:
				format = GL_R8;
				break;
			case TextureBit8:
				format = GL_R8;
				break;
			case TextureBit16:
				format = GL_R16;
				break;
			case TextureBit32:
				format = GL_RED;
				break;
			default:
				format = GL_R16;
				break;
			}
			break;
		case TextureDataInt:
			switch (m_bitSize) {
			case TextureBit4:
				format = GL_R8I;
				break;
			case TextureBit8:
				format = GL_R8I;
				break;
			case TextureBit16:
				format = GL_R16I;
				break;
			case TextureBit32:
				format = GL_R32I;
				break;
			default:
				format = GL_R16I;
				break;
			}
			break;
		default:
			format = GL_R;
			break;
		}
	}

	return format;
}

unsigned int Texture::GetFormat() const {
	unsigned int format = m_use;

	if (m_use == TextureUseColor) {
		switch (m_dataType) {
		case TextureDataUInt:
			format = GL_RGBA_INTEGER;
			break;
		case TextureDataInt:
			format = GL_RGBA_INTEGER;
			break;
		default:
			format = GL_RGBA;
			break;
		}
	}
	else if (m_use == TextureUseDepth) {
		format = GL_DEPTH_COMPONENT;
	}
	else if (m_use == TextureUseGrayscale) {
		switch (m_dataType) {
		case TextureDataUInt:
			format = GL_RED_INTEGER;
			break;
		case TextureDataInt:
			format = GL_RED_INTEGER;
			break;
		default:
			format = GL_RED;
			break;
		}
	}

	return format;
}

void Texture::glCreateTexture() {
	if (m_textureID) {
		glDeleteTextures(1, &m_textureID);
	}

	glGenTextures(1, &m_textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(m_type, m_textureID);

	SetTextureMinFilter(m_minFilter);
	SetTextureMagFilter(m_magFilter);

	SetTextureWrap(m_wrapType);
}

void Texture::SetTextureData() {
	Bind(0);
	unsigned int internal = GetInternalFormat();
	unsigned int format = GetFormat();

	if (m_use == TextureUseGrayscale || m_use == TextureUseDepth) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
	}

	switch (m_type) {
	case TextureType2D:
		glTexImage2D(GL_TEXTURE_2D, 0, internal, m_resolution.x, m_resolution.y, 0, format, m_dataType, nullptr);
		break;
	case TextureTypeCubemap:
		for (int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal, m_resolution.x, m_resolution.y, 0, format, m_dataType, nullptr);
		}
		break;
	}

	Unbind();
}

Texture2D::Texture2D(const filesystem::path& file, const TextureDataType dataType, const TextureUse use, const TextureBitSize bitSize)
	: Texture({1, 1}, TextureType2D, dataType, use, bitSize), m_filePath(file) {
	LoadTexture(file);
}

Texture2D::Texture2D(const int2 resolution, const TextureDataType dataType, const TextureUse use, const TextureBitSize bitSize)
	: Texture(resolution, TextureType2D, dataType, use, bitSize) {

}

bool Texture2D::LoadTexture(const filesystem::path& file) {
	if (!filesystem::exists(file)) {
		GlenderLog(LogTypeError, format("Texture file does not exist: {}", file.string()));
		return false;
	}

	m_filePath = file;

	stbi_set_flip_vertically_on_load(true);

	Bind(0);
	
	unsigned char* bytes = stbi_load(file.string().c_str(), &m_resolution.x, &m_resolution.y, &m_channels, 4);


	unsigned int internal = GetInternalFormat();
	unsigned int format = GL_RGBA;
	if (m_dataType == TextureDataInt || m_dataType == TextureDataUInt) {
		format = GL_RGBA_INTEGER;
	}
	if (m_use == TextureUseGrayscale || m_use == TextureUseDepth) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internal, m_resolution.x, m_resolution.y, 0, format, GL_UNSIGNED_BYTE, bytes);

	Unbind();

	stbi_image_free(bytes);

	return true;
}

void Texture2D::SetTexturePath(const filesystem::path& path) {
	m_filePath = path;
}

filesystem::path Texture2D::GetTexturePath() {
	return m_filePath;
}

Cubemap::Cubemap(unordered_map<CubemapSides, filesystem::path> paths, const TextureDataType dataType, 
	const TextureUse use, const TextureBitSize bitSize)
	: Texture({1, 1}, TextureTypeCubemap, dataType, use, bitSize), m_filePaths(paths) {

	for (pair<const CubemapSides, filesystem::path>& file : paths) {
		LoadTexture(file.second, file.first);
	}
}

Cubemap::Cubemap(const int2 resolution, const TextureDataType dataType, const TextureUse use, const TextureBitSize bitSize)
	: Texture(resolution, TextureTypeCubemap, dataType, use, bitSize) {

}

bool Cubemap::LoadTexture(const filesystem::path& file, const CubemapSides side) {
	if (!filesystem::exists(file)) {
		GlenderLog(LogTypeError, format("Texture file does not exist: {}", file.string()));
		return false;
	}

	m_filePaths[side] = file;

	stbi_set_flip_vertically_on_load(false);

	Bind(0);

	unsigned char* bytes = stbi_load(file.string().c_str(), &m_resolution.x, &m_resolution.y, &m_channels, 4);

	unsigned int internal = GetInternalFormat();
	unsigned int format = GL_RGBA;
	if (m_dataType == TextureDataInt || m_dataType == TextureDataUInt) {
		format = GL_RGBA_INTEGER;
	}
	if (m_use == TextureUseGrayscale || m_use == TextureUseDepth) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
	}

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, internal, m_resolution.x, m_resolution.y, 0, format, GL_UNSIGNED_BYTE, bytes);

	Unbind();

	stbi_image_free(bytes);

	return true;
}

filesystem::path Cubemap::GetTexturePath() {
	return m_filePaths[CubemapFront];
}

filesystem::path Cubemap::GetTexturePath(const CubemapSides side) {
	return m_filePaths[side];
}

FontMap::FontMap(const int2 mapSize, const filesystem::path& font) 
		: Texture(mapSize, TextureType2D, TextureDataUByte, TextureUseColor, TextureBit32), m_fontPath(font) {
	SetTextureMagFilter(TextureMagFilterLinear);
	SetTextureMinFilter(TextureMinFilterLinearMipNearest);

	LoadFont(font);

	Bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
	Unbind();
}

filesystem::path FontMap::GetTexturePath() {
	return m_fontPath;
}

FontGlyphData FontMap::GetGlyphData(const char glyph) {
	return m_glyphs[glyph];
}

void FontMap::LoadFont(const filesystem::path& font) {
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		GlenderLog(LogTypeError, "Failed to initialize FreeType.");
		return;
	}
	
	string filepath = font.string();
	FT_Face face;
	if (FT_New_Face(ft, filepath.c_str(), 0, &face)) {
		GlenderLog(LogTypeError, format("Failed to load font: {}", m_fontPath.string()));
		return;
	}
	
	FT_Select_Charmap(face, FT_ENCODING_UNICODE);

	m_fontPath = font;

	shared_ptr<Material> sdfMat = make_shared<Material>(MaterialTypeNormal, 
		vector<filesystem::path>{ "shaders/Quad.vert", "shaders/Text/FontSdf.frag" });

	OverlayMesh quad(sdfMat);
	Framebuffer sdfRender = Framebuffer(m_resolution);
	sdfRender.AddTexture(this);

	GenerateMipmaps();

	Texture glyphTmp = Texture(int2{32, 32}, TextureType2D, TextureDataUByte, TextureUseGrayscale);
	glyphTmp.SetTextureWrap(TextureWrapClamp);

	FontGlyphData glyphData = {};

	FT_Long num = face->num_glyphs;

	float glyphHeight = max(m_resolution.x / sqrt(num * m_resolution.x / m_resolution.y),
								m_resolution.y / sqrt(num * m_resolution.y / m_resolution.x));

	FT_Set_Pixel_Sizes(face, 0, 512);

	FT_UInt index;
	FT_ULong character = FT_Get_First_Char(face, &index);
	
	lm::vec2 start = { 0, glyphHeight };
	lm::vec2 end = {};


	sdfRender.Bind();
	sdfRender.Clear({0, 0, 0, 1});

	float scale = glyphHeight / 512;
	float nextY = 0;
	int mips = 5;

	while (index) {
		FT_Load_Char(face, character, FT_LOAD_RENDER);

		lm::vec2 glyphSize = { (float)face->glyph->bitmap.width, (float)face->glyph->bitmap.rows };
		if (glyphSize.x <= 0 || glyphSize.y <= 0) {
			character = FT_Get_Next_Char(face, character, &index);
			continue;
		}
		glyphTmp.SetData(face->glyph->bitmap.buffer, { (int)glyphSize.x, (int)glyphSize.y });

		sdfMat->SetUniform("u_TexSize", glyphSize);
		glyphSize *= scale;

		float scaleTmp = scale;

		if (start.x + glyphSize.x > m_resolution.x) {
			start = { 0, start.y + glyphHeight };
			nextY = 0;
		}
		end = { start.x + glyphSize.x, start.y + glyphSize.y };

		lm::vec2 currRes = glyphSize;
		lm::vec2 currStart = start;
		for (int mip = 0; currRes.x >= 1 && currRes.y >= 1; mip++) {
			sdfRender.ChangeMipLevel(this, mip);
			sdfRender.Bind();

			glViewport(currStart.x, currStart.y, currRes.x, currRes.y);

			glyphTmp.Bind(0);
			quad.RenderMesh();

			scaleTmp /= 2;
			currRes /= 2;
			currStart /= 2;
		}

		if (nextY < face->glyph->bitmap_top) {
			nextY = face->glyph->bitmap_top;
		}

		glyphData.Glyph = character;
		glyphData.UVMin = start;
		glyphData.UVMin /= lm::vec2(m_resolution.x, m_resolution.y);
		glyphData.UVMax = end / lm::vec2(m_resolution.x, m_resolution.y);
		glyphData.Bearing = (glyphSize.y - face->glyph->bitmap_top * scale) / m_resolution.y;

		m_glyphs[character] = glyphData;

		start.x += glyphHeight;

		character = FT_Get_Next_Char(face, character, &index);
	}

	sdfRender.Unbind();

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}
