#pragma once
#include "Component.h"
#include "Model.h"
//#include "Shader.h"
namespace Engine {
	class ComponentGeometry : public Component
	{
	public:
		ComponentGeometry(PremadeModel modelType, const char* vShaderFilepath, const char* fShaderFilepath);
		ComponentGeometry(PremadeModel modelType);
		ComponentGeometry(const char* modelFilepath, const char* vShaderFilepath, const char* fShaderFilepath, bool pbr);
		ComponentGeometry(const char* modelFilepath, bool pbr);
		~ComponentGeometry();

		ComponentTypes ComponentType() override { return COMPONENT_GEOMETRY; }
		void Close() override;

		bool PBR() { return pbr; }

		void SetCulling(bool cull, GLenum cull_type) { CULL_FACE = cull, CULL_TYPE = cull_type; }
		void SetShader(Shader* newShader) { shader = newShader; }

		bool Cull_Face() { return CULL_FACE; }
		GLenum Cull_Type() { return CULL_TYPE; }

		Model* GetModel() { return model; }
		Shader* GetShader() { return shader; }

		float GetTextureScale() { return textureScale; }
		
		void CastShadows(bool shadows) { castShadows = shadows; }
		bool CastShadows() { return castShadows; }

		void SetTextureScale(float newScale) { textureScale = newScale; }
	private:
		Model* model;
		Shader* shader;

		float textureScale;
		bool castShadows;

		bool pbr;
		bool usingDefaultShader;

		GLenum CULL_TYPE;
		bool CULL_FACE;
	};
}