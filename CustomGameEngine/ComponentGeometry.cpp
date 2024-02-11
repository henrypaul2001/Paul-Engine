#include "ComponentGeometry.h"
namespace Engine {
	ComponentGeometry::ComponentGeometry(const char* modelFilepath, const char* vShaderFilepath, const char* fShaderFilepath, bool pbr)
	{
		this->pbr = pbr;
		model = new Model(modelFilepath, pbr);
		shader = new Shader(vShaderFilepath, fShaderFilepath);
	}

	ComponentGeometry::ComponentGeometry(const char* modelFilepath, bool pbr)
	{
		this->pbr = pbr;
		model = new Model(modelFilepath, pbr);
		std::string v;
		std::string f;
		if (pbr) {
			v = "default_pbr.vert";
			f = "default_pbr.frag";
		}
		else {
			v = "default.vert";
			f = "default.frag";
		}
		shader = new Shader(v.c_str(), f.c_str());
	}

	ComponentGeometry::~ComponentGeometry()
	{
		delete shader;
		delete model;
	}

	void ComponentGeometry::Close()
	{

	}
}