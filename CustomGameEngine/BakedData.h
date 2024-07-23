#pragma once
#include <glm/ext/vector_float3.hpp>
#include "ReflectionProbe.h"
#include <vector>
namespace Engine {
	class BakedData
	{
	public:
		BakedData() {}
		~BakedData() {
			ClearReflectionProbes();
		}

		void InitialiseReflectionProbes(const std::vector<glm::vec3>& positions) {
			ClearReflectionProbes();

			for (const glm::vec3& position : positions) {
				reflectionProbes.push_back(new ReflectionProbe(reflectionProbes.size(), position));
			}
		}

	private:
		void ClearReflectionProbes() {
			for (ReflectionProbe* probe : reflectionProbes) {
				delete probe;
			}
			reflectionProbes.clear();
		}

		std::vector<ReflectionProbe*> reflectionProbes;
	};
}