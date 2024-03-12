#include "ComponentGeometry.h"
#include "SceneManager.h"
namespace Engine {
	ComponentGeometry::ComponentGeometry(PremadeModel modelType, const char* vShaderFilepath, const char* fShaderFilepath)
	{
		this->pbr = false;
		model = new Model(modelType);

		CULL_FACE = true;
		CULL_TYPE = GL_BACK;

		if (modelType == MODEL_PLANE) {
			//CULL_FACE = false;
			CULL_TYPE = GL_BACK;
		}

		usingDefaultShader = false;

		castShadows = true;

		textureScale = 1.0f;

		shader = ResourceManager::GetInstance()->LoadShader(vShaderFilepath, fShaderFilepath);
	}

	ComponentGeometry::ComponentGeometry(PremadeModel modelType)
	{
		this->pbr = false;
		model = new Model(modelType);

		CULL_FACE = true;
		CULL_TYPE = GL_BACK;

		if (modelType == MODEL_PLANE) {
			//CULL_FACE = false;
			CULL_TYPE = GL_BACK;
		}

		usingDefaultShader = true;

		castShadows = true;

		shader = nullptr;
		if (RenderManager::GetInstance()->GetRenderPipeline()->Name() == FORWARD_PIPELINE) {
			shader = ResourceManager::GetInstance()->DefaultLitShader();
		}
		else if (RenderManager::GetInstance()->GetRenderPipeline()->Name() == DEFERRED_PIPELINE) {
			shader = ResourceManager::GetInstance()->DeferredGeometryPass();
		}

		textureScale = 1.0f;
	}

	ComponentGeometry::ComponentGeometry(const char* modelFilepath, const char* vShaderFilepath, const char* fShaderFilepath, bool pbr)
	{
		this->pbr = pbr;

		model = ResourceManager::GetInstance()->LoadModel(modelFilepath, pbr);

		usingDefaultShader = false;

		castShadows = true;

		textureScale = 1.0f;

		CULL_FACE = true;
		CULL_TYPE = GL_BACK;

		shader = ResourceManager::GetInstance()->LoadShader(vShaderFilepath, fShaderFilepath);
	}

	ComponentGeometry::ComponentGeometry(const char* modelFilepath, bool pbr)
	{
		CULL_FACE = true;
		CULL_TYPE = GL_BACK;

		this->pbr = pbr;

		usingDefaultShader = true;

		castShadows = true;

		model = ResourceManager::GetInstance()->LoadModel(modelFilepath, pbr);

		shader = nullptr;
		if (pbr) {
			shader = ResourceManager::GetInstance()->DefaultLitPBR();
		}
		else {
			if (RenderManager::GetInstance()->GetRenderPipeline()->Name() == FORWARD_PIPELINE) {
				shader = ResourceManager::GetInstance()->DefaultLitShader();
			}
			else if (RenderManager::GetInstance()->GetRenderPipeline()->Name() == DEFERRED_PIPELINE) {
				shader = ResourceManager::GetInstance()->DeferredGeometryPass();
			}
		}

		textureScale = 1.0f;
	}

	ComponentGeometry::~ComponentGeometry()
	{

	}

	void ComponentGeometry::Close()
	{

	}
}