#pragma once
#include "PaulEngine/Asset/Asset.h"
#include "Shader.h"

namespace PaulEngine
{
	/*
		Material:
			AssetHandle shader;
			ShaderParameters params;

		ShaderParameters:
			virtual void Bind() = 0;

		OpaquePBRParamaters : ShaderParameters:
			Albedo, Roughness, Metalness;
			ColourMaps;
			DisplacementMap;

			virtual void Bind() override {};
	*/

	class ShaderParameters
	{
	public:
		virtual void Bind(Ref<Shader> shader) const = 0;
	};

	class FlatColourShaderParameters : public ShaderParameters
	{
	public:
		glm::vec4 m_Colour;

		virtual void Bind(Ref<Shader> shader) const override {
			shader->SetUniformFloat4("u_Colour", m_Colour);
		}
	};

	class Material : public Asset
	{
	public:
		Material();

		void Bind();

		virtual AssetType GetType() const { return AssetType::Material; }

	private:
		Ref<Shader> m_Shader;
		Ref<ShaderParameters> m_Paramaters;
	};
}