#include "pepch.h"
#include "Material.h"

namespace PaulEngine
{
	Material::Material()
	{

	}

	void Material::Bind()
	{
		m_Shader->Bind();
		//m_Paramaters->Bind(m_Shader);
	}
}