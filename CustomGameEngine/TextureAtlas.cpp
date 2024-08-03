#include "TextureAtlas.h"
namespace Engine {
	TextureAtlas::TextureAtlas(const unsigned int rows, const unsigned int columns, const unsigned int slotWidth, const unsigned int slotHeight, const GLenum internalFormat, const GLenum format, const GLenum type)
	{
		this->internalFormat = internalFormat;
		this->format = format;
		this->type = type;

		textureID = 0;
		glGenTextures(1, &textureID);

		ResizeTexture(rows, columns, slotWidth, slotHeight);
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

	void TextureAtlas::InitialiseTexture()
	{
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColour[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour);
	
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}