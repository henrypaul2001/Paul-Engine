#pragma once
#include "TextureAtlas.h"
namespace Engine {
	class FlatTextureAtlas : public TextureAtlas
	{
	public:
		FlatTextureAtlas(const unsigned int rows, const unsigned int columns, const unsigned int slotWidth, const unsigned int slotHeight, const GLenum internalFormat = GL_RGBA, const GLenum format = GL_RGBA, const GLenum type = GL_UNSIGNED_BYTE,
			const GLenum minFilter = GL_NEAREST, const GLenum magFilter = GL_NEAREST, const GLenum wrapS = GL_REPEAT, const GLenum wrapT = GL_REPEAT, const glm::vec4& borderColour = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
		~FlatTextureAtlas() {}

	protected:
		void InitialiseTexture() override;
	};
}
