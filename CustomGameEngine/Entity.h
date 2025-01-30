#pragma once
#include <string>
#include <bitset>
namespace Engine {
	static constexpr unsigned int MAX_COMPONENTS = 32;
	class Entity {
	public:
		friend class EntityManager;

		~Entity() {}

		const std::string& Name() const { return name; }
		const unsigned int ID() const { return id; }

	protected:
		Entity(const std::string& name, const unsigned int id);

	private:
		std::string name;
		unsigned int id;
		std::bitset<MAX_COMPONENTS> component_mask;
	};
}