#pragma once

#include <xhash>

namespace PaulEngine
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};
}

// Hash template specialisation
namespace std
{
	template<>
	struct hash<PaulEngine::UUID>
	{
		std::size_t operator()(const PaulEngine::UUID& uuid) const {
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}