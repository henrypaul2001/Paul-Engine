#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
namespace Engine {
	class TextureAtlas
	{
	public:
		TextureAtlas(const unsigned int rows, const unsigned int columns, const unsigned int slotWidth, const unsigned int slotHeight, const GLenum internalFormat = GL_RGBA, const GLenum format = GL_RGBA, const GLenum type = GL_UNSIGNED_BYTE, 
								const GLenum minFilter = GL_NEAREST, const GLenum magFilter = GL_NEAREST, const GLenum wrapS = GL_REPEAT, const GLenum wrapT = GL_REPEAT, const glm::vec4& borderColour = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
		~TextureAtlas();

		const unsigned int GetTextureID() const { return textureID; }
		const unsigned int GetSlotWidth() const { return slotWidth; }
		const unsigned int GetSlotHeight() const { return slotHeight; }
		const unsigned int GetNumRows() const { return numRows; }
		const unsigned int GetNumColumns() const { return numColumns; }
		const unsigned int GetWidth() const { return width; }
		const unsigned int GetHeight() const { return height; }

		const glm::uvec2& GetSlotStartXY(const unsigned int row, const unsigned int column) const { return glm::uvec2(column * slotWidth, row * slotHeight); }

		void ResizeTexture(const unsigned int rows, const unsigned int columns, const unsigned int slotWidth, const unsigned int slotHeight);
		void ResizeTextureResolution(const unsigned int newWidth, const unsigned int newHeight);
		void ResizeTextureDimensions(const unsigned int newRows, const unsigned int newColumns);
		void ResizeTextureSlotResolution(const unsigned int newSlotWidth, const unsigned int newSlotHeight);

	protected:
		virtual void InitialiseTexture() = 0;

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
		GLenum minFilter;
		GLenum magFilter;
		GLenum wrapS;
		GLenum wrapT;
		glm::vec4 borderColour;
	};
}