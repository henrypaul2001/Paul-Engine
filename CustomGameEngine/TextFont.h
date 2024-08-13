#pragma once
#include <map>
#include <glm/ext/vector_int2.hpp>
#include <glad/glad.h>
namespace Engine {
	struct TextCharacter {
		unsigned int TextureID; // glyph texture
		glm::ivec2 Size;		// size of glyph
		glm::ivec2 Bearing;		// offset from baseline to left/top of glyph
		unsigned int Advance;	// offset to next glyph
	};

	class TextFont
	{
	public:
		TextFont() {}
		~TextFont() {
			for (std::map<char, TextCharacter>::iterator characterIt = characters.begin(); characterIt != characters.end(); characterIt++) {
				glDeleteTextures(1, &characterIt->second.TextureID);
			}
		}

		const TextCharacter* GetCharacter(char c) const;
		void AddCharacter(char c, const TextCharacter& character) { characters.insert(std::pair<char, TextCharacter>(c, character)); }
		const std::map<char, TextCharacter>& GetCharacters() const { return characters; }
	private:
		std::map<char, TextCharacter> characters;
	};
}