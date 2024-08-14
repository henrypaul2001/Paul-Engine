#include "CubeTextureAtlas.h"
namespace Engine {
	CubeTextureAtlas::CubeTextureAtlas(const unsigned int slotsPerFace, const unsigned int slotWidth, const unsigned int slotHeight, const GLenum internalFormat,
		const GLenum format, const GLenum type, const GLenum minFilter, const GLenum magFilter, const GLenum wrapS, const GLenum wrapT, const GLenum wrapR, const glm::vec4& borderColour) : TextureAtlas(slotsPerFace / 2, slotsPerFace / 2, slotWidth, slotHeight, internalFormat, format, type, minFilter, magFilter, wrapS, wrapT, borderColour)
	{
		this->wrapR = wrapR;
		ResizeTexture(slotsPerFace / 2, slotsPerFace / 2, slotWidth, slotHeight);
	}

	void CubeTextureAtlas::InitialiseTexture()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		// for each side of cubemap
		for (unsigned int i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, type, 0);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapR);
		float colour[] = { borderColour.r, borderColour.g, borderColour.b, borderColour.a };
		glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, colour);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
}