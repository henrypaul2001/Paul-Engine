#pragma once
#include <string>
#include <bitset>
namespace Engine {
	static constexpr unsigned int MAX_COMPONENTS = 13;
	class EntityNew {
	public:
		friend class EntityManagerNew;

		~EntityNew() {}

		const std::string& Name() const { return name; }
		const unsigned int ID() const { return id; }

	protected:
		EntityNew(const std::string& name, const unsigned int id);

	private:
		std::string name;
		unsigned int id;
		std::bitset<MAX_COMPONENTS> component_mask;
	};
}