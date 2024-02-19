#pragma once
#include "Component.h"
#include "Model.h"
//#include "Shader.h"
namespace Engine {
	class ComponentGeometry : public Component
	{
	public:
		ComponentGeometry(const char* modelFilepath, const char* vShaderFilepath, const char* fShaderFilepath, bool pbr);
		ComponentGeometry(const char* modelFilepath, bool pbr);
		~ComponentGeometry();

		ComponentTypes ComponentType() override { return COMPONENT_GEOMETRY; }
		void Close() override;

		bool PBR() { return pbr; }

		Model* GetModel() { return model; }
		Shader* GetShader() { return shader; }
		float GetTextureScale() { return textureScale; }
		
		void SetTextureScale(float newScale) { textureScale = newScale; }
	private:
		Model* model;
		Shader* shader;

		float textureScale;

		bool pbr;
		bool usingDefaultShader;
	};
}