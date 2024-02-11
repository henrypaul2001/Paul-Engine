#pragma once
#include "Component.h"
namespace Engine {
	class ComponentGeometry : Component
	{
	public:
		ComponentGeometry(const char* modelFilepath, const char* vShaderFilepath, const char* fShaderFilepath);
		ComponentGeometry(const char* modelFilepath);
		~ComponentGeometry();

		ComponentTypes ComponentType() override { return COMPONENT_GEOMETRY; }
		void Close() override;

		bool PBR() { return pbr; }

		// Model* GetModel() { return model; }
		// Shader& GetShader() { return shader; }
	private:
		// Shader shader
		// Model model
		bool pbr;
	};
}