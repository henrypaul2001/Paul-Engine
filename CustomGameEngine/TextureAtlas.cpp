#include "TextureAtlas.h"
namespace Engine {
	TextureAtlas::TextureAtlas(const unsigned int rows, const unsigned int columns, const unsigned int slotWidth, const unsigned int slotHeight, const GLenum internalFormat, 
										const GLenum format, const GLenum type, const GLenum minFilter, const GLenum magFilter, const GLenum wrapS, const GLenum wrapT, const glm::vec4& borderColour)
	{
		this->internalFormat = internalFormat;
		this->format = format;
		this->type = type;
		this->minFilter = minFilter;
		this->magFilter = magFilter;
		this->wrapS = wrapS;
		this->wrapT = wrapT;
		this->borderColour = borderColour;

		textureID = 0;
		glGenTextures(1, &textureID);
	}

	TextureAtlas::~TextureAtlas()
	{
		glDeleteTextures(1, &textureID);
	}

	void TextureAtlas::ResizeTexture(const unsigned int rows, const unsigned int columns, const unsigned int slotWidth, const unsigned int slotHeight)
	{
		this->numRows = rows;
		this->numColumns = columns;

		if (numRows < 1) { numRows = 1; }
		if (numColumns < 1) { numColumns = 1; }

		this->slotWidth = slotWidth;
		this->slotHeight = slotHeight;

		width = numColumns * slotWidth;
		height = numRows * slotHeight;

		InitialiseTexture();
	}

	void TextureAtlas::ResizeTextureResolution(const unsigned int newWidth, const unsigned int newHeight)
	{
		width = newWidth;
		height = newHeight;

		slotWidth = width / numColumns;
		slotHeight = height / numRows;

		InitialiseTexture();
	}

	void TextureAtlas::ResizeTextureDimensions(const unsigned int newRows, const unsigned int newColumns)
	{
		numRows = newRows;
		numColumns = newColumns;

		if (numRows < 1) { numRows = 1; }
		if (numColumns < 1) { numColumns = 1; }

		width = numColumns * slotWidth;
		height = numRows * slotHeight;

		InitialiseTexture();
	}

	void TextureAtlas::ResizeTextureSlotResolution(const unsigned int newSlotWidth, const unsigned int newSlotHeight)
	{
		slotWidth = newSlotWidth;
		slotHeight = newSlotHeight;

		width = numColumns * slotWidth;
		height = numRows * slotHeight;
	
		InitialiseTexture();
	}
}