#include "FlatTextureAtlas.h"
namespace Engine {
	FlatTextureAtlas::FlatTextureAtlas(const unsigned int rows, const unsigned int columns, const unsigned int slotWidth, const unsigned int slotHeight, const GLenum internalFormat, const GLenum format, const GLenum type, 
			const GLenum minFilter, const GLenum magFilter, const GLenum wrapS, const GLenum wrapT, const glm::vec4& borderColour) : TextureAtlas(rows, columns, slotWidth, slotHeight, internalFormat, format, type, minFilter, magFilter, wrapS, wrapT, borderColour)
	{
		ResizeTexture(rows, columns, slotWidth, slotHeight);
	}

	void FlatTextureAtlas::InitialiseTexture()
	{
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		float colour[] = { borderColour.r, borderColour.g, borderColour.b, borderColour.a };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, colour);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}