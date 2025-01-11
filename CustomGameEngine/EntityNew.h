#pragma once
#include <string>
namespace Engine {
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
	};
}