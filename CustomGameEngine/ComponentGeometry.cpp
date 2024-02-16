#include "ComponentGeometry.h"
#include "SceneManager.h"
namespace Engine {
	ComponentGeometry::ComponentGeometry(const char* modelFilepath, const char* vShaderFilepath, const char* fShaderFilepath, bool pbr)
	{
		this->pbr = pbr;
		model = new Model(modelFilepath, pbr); // dont do this. Use resource manager to avoid duplicates and loading models during gameplay
		usingDefaultShader = false;
		//shader = new Shader(vShaderFilepath, fShaderFilepath); // dont do this. Use resource manager instead to avoid duplicate shaders, also do it at start instead of potentially loading shaders during gameplay
	}

	ComponentGeometry::ComponentGeometry(const char* modelFilepath, bool pbr)
	{
		this->pbr = pbr;
		usingDefaultShader = true;
		model = ResourceManager::GetInstance()->LoadModel(modelFilepath, pbr);
		std::string v;
		std::string f;
		if (pbr) {
			v = "Shaders/defaultLit_pbr.vert";
			f = "Shaders/defaultLit_pbr.frag";
			//shader = new Shader(v.c_str(), f.c_str()); // dont do this. Use resource manager instead to avoid duplicate shaders, also do it at start instead of potentially loading shaders during gameplay
		}
		else {
			//v = "Shaders/defaultLit.vert";
			//f = "Shaders/defaultLit.frag";
			shader = SceneManager::defaultLit;
		}
		
		// configure shader uniforms
	}

	ComponentGeometry::~ComponentGeometry()
	{
		//delete shader;
		//delete model;
	}

	void ComponentGeometry::Close()
	{

	}
}