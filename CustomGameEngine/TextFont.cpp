#include "TextFont.h"
namespace Engine {
	const TextCharacter* TextFont::GetCharacter(char c) const
	{
		std::map<char, TextCharacter>::const_iterator it = characters.find(c);
		if (it != characters.end()) {
			return &it->second;
		}
		else {
			return nullptr;
		}
	}
}