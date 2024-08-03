#pragma once
#include <glad/glad.h>
namespace Engine {
	class TextureAtlas
	{
	public:
		TextureAtlas(const unsigned int rows = 2, const unsigned int columns = 2, const unsigned int slotWidth = 256, const unsigned int slotHeight = 256, const GLenum internalFormat = GL_RGBA, const GLenum format = GL_RGBA, const GLenum type = GL_UNSIGNED_BYTE);
		~TextureAtlas() {
			glDeleteTextures(1, &textureID);
		}

		const unsigned int GetTextureID() const { return textureID; }

		void ResizeTexture(const unsigned int rows, const unsigned int columns, const unsigned int slotWidth, const unsigned int slotHeight);
		void ResizeTextureResolution(const unsigned int newWidth, const unsigned int newHeight);
		void ResizeTextureDimensions(const unsigned int newRows, const unsigned int newColumns);
		void ResizeTextureSlotResolution(const unsigned int newSlotWidth, const unsigned int newSlotHeight);
	private:
		void InitialiseTexture();

		unsigned int numRows;
		unsigned int numColumns;

		unsigned int width;
		unsigned int height;

		unsigned int slotWidth;
		unsigned int slotHeight;

		unsigned int textureID;

		GLenum internalFormat;
		GLenum format;
		GLenum type;
	};
}