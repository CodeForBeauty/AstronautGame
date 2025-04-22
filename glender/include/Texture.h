#pragma once

#include "Types.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <unordered_map>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H 


namespace glender {
	enum TextureDataType {
		TextureDataUInt = GL_UNSIGNED_INT,
		TextureDataFloat = GL_FLOAT,
		TextureDataUByte = GL_UNSIGNED_BYTE,
		TextureDataInt = GL_INT,
	};

	enum TextureType {
		TextureType2D = GL_TEXTURE_2D,
		TextureTypeCubemap = GL_TEXTURE_CUBE_MAP
	};

	enum TextureUse {
		TextureUseColor = GL_RGBA,
		TextureUseDepth = GL_DEPTH_COMPONENT,
		TextureUseGrayscale = GL_RED
	};

	enum TextureBitSize {
		TextureBit4,
		TextureBit8,
		TextureBit16,
		TextureBit32,
	};

	enum TextureResizeType {
		TextureResizeClear,
		TextureResizeAdd,
		TextureResizeSample
	};

	enum TextureWrapType {
		TextureWrapClamp = GL_CLAMP_TO_EDGE,
		TextureWrapRepeat = GL_REPEAT,
		TextureWrapMirroredRepeat = GL_MIRRORED_REPEAT,
		TextureWrapMirroredClamp = GL_MIRROR_CLAMP_TO_EDGE,
	};

	enum TextureMagFilterType {
		TextureMagFilterNearest = GL_NEAREST,
		TextureMagFilterLinear = GL_LINEAR,
	};

	enum TextureMinFilterType {
		TextureMinFilterNearest = GL_NEAREST,
		TextureMinFilterLinear = GL_LINEAR,
		TextureMinFilterNearestMipNearest = GL_NEAREST_MIPMAP_NEAREST,
		TextureMinFilterLinearMipLinear = GL_LINEAR_MIPMAP_LINEAR,
		TextureMinFilterNearestMipLinear = GL_NEAREST_MIPMAP_LINEAR,
		TextureMinFilterLinearMipNearest = GL_LINEAR_MIPMAP_NEAREST,
	};

	class Texture {
	public:
		Texture(const int2 resolution, const TextureType type = TextureType2D, const TextureDataType dataType = TextureDataUByte, 
			const TextureUse use = TextureUseColor, const TextureBitSize bitSize = TextureBit32);
		~Texture();


		void Bind(unsigned int slot);
		void Unbind();

		void BindToFramebuffer(const int index, int mip = 0);

		virtual std::filesystem::path GetTexturePath() { return ""; };

		void ResizeTexture(const TextureResizeType resizeType, const int2 newSize);
		void SetData(const void* data, const int2 newSize);
		int2 GetSize() const;

		TextureUse GetUse() const;

		void SetTextureWrap(const TextureWrapType type);
		void SetTextureMinFilter(const TextureMinFilterType type);
		void SetTextureMagFilter(const TextureMagFilterType type);

		void GenerateMipmaps();

		void ReadPixel(unsigned int x, unsigned int y, void* data);

	protected:
		unsigned int m_textureID;

		int2 m_resolution;
		int m_channels;

		TextureType m_type;
		TextureDataType m_dataType;
		TextureUse m_use;
		TextureBitSize m_bitSize;

		TextureWrapType m_wrapType;
		TextureMinFilterType m_minFilter;
		TextureMagFilterType m_magFilter;

		unsigned int GetInternalFormat() const;
		unsigned int GetFormat() const;

	private:
		unsigned int m_lastSlot;

		void glCreateTexture();

		void SetTextureData();
	};

	class Texture2D : public Texture {
	public:
		Texture2D(const std::filesystem::path& file, const TextureDataType dataType = TextureDataUByte, 
			const TextureUse use = TextureUseColor, const TextureBitSize bitSize = TextureBit32);
		Texture2D(const int2 resolution, const TextureDataType dataType = TextureDataUByte, 
			const TextureUse use = TextureUseColor, const TextureBitSize bitSize = TextureBit32);

		bool LoadTexture(const std::filesystem::path& file);
		void SetTexturePath(const std::filesystem::path& path);

		std::filesystem::path GetTexturePath() override;

	protected:
		std::filesystem::path m_filePath;

	};

	enum CubemapSides {
		CubemapFront,
		CubemapBack,
		CubemapTop,
		CubemapBottom,
		CubemapRight,
		CubemapLeft,
	};

	class Cubemap : public Texture {
	public:
		Cubemap(std::unordered_map<CubemapSides, std::filesystem::path> paths, const TextureDataType dataType = TextureDataUByte,
			const TextureUse use = TextureUseColor, const TextureBitSize bitSize = TextureBit32);
		Cubemap(const int2 resolution, const TextureDataType dataType = TextureDataUByte, 
			const TextureUse use = TextureUseColor, const TextureBitSize bitSize = TextureBit32);

		bool LoadTexture(const std::filesystem::path& file, const CubemapSides side);

		std::filesystem::path GetTexturePath() override;
		std::filesystem::path GetTexturePath(const CubemapSides side);

	protected:
		std::unordered_map<CubemapSides, std::filesystem::path> m_filePaths;
	};

	
	struct FontGlyphData {
	public:
		char Glyph;

		lm::vec2 UVMin;
		lm::vec2 UVMax;

		float Bearing;
	};

	class FontMap : public Texture {
	public:
		FontMap(const int2 mapSize, const std::filesystem::path& font);

		std::filesystem::path GetTexturePath() override;

		FontGlyphData GetGlyphData(const char glyph);

	protected:
		std::map<unsigned long, FontGlyphData> m_glyphs;

		std::filesystem::path m_fontPath;

		void LoadFont(const std::filesystem::path& font);

	private:
		
	};
}