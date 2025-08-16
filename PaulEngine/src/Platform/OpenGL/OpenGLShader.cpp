#include "pepch.h"
#include "OpenGLShader.h"
#include "PaulEngine/Renderer/Resource/ShaderParameterType.h"

#include <glad/glad.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "spirv_reflect.h"

namespace PaulEngine {

	namespace OpenGLShaderUtils
	{
		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex") { return GL_VERTEX_SHADER; }
			if (type == "fragment" || type == "pixel") { return GL_FRAGMENT_SHADER; }
			if (type == "geometry") { return GL_GEOMETRY_SHADER; }
			PE_CORE_ASSERT(false, "Unknown shader type");
			return 0;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER: return shaderc_glsl_vertex_shader;
				case GL_FRAGMENT_SHADER: return shaderc_glsl_fragment_shader;
				case GL_GEOMETRY_SHADER: return shaderc_glsl_geometry_shader;
			}
			PE_CORE_ASSERT(false, "Unknown shader stage");
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER: return "GL_VERTEX_SHADER";
				case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
				case GL_GEOMETRY_SHADER: return "GL_GEOMETRY_SHADER";
			}
			PE_CORE_ASSERT(false, "Unknown shader stage");
			return "";
		}

		static const char* GetCacheDirectory()
		{
			return "assets/cache/shader/opengl";
		}

		static void ValidateCacheDirectory()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!std::filesystem::exists(cacheDirectory))
			{
				PE_CORE_DEBUG("Creating directory '{0}'", cacheDirectory);
				std::filesystem::create_directories(cacheDirectory);
			}
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER: return ".cached_opengl.vert";
				case GL_FRAGMENT_SHADER: return ".cached_opengl.frag";
				case GL_GEOMETRY_SHADER: return ".cached_opengl.geom";
			}
			PE_CORE_ASSERT(false, "Unknown shader stage");
			return "";
		}

		static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER: return ".cached_vulkan.vert";
				case GL_FRAGMENT_SHADER: return ".cached_vulkan.frag";
				case GL_GEOMETRY_SHADER: return ".cached_vulkan.geom";
			}
			PE_CORE_ASSERT(false, "Unknown shader stage");
			return "";
		}

		static ShaderDataType SpirTypeToShaderDataType(spirv_cross::SPIRType spirType)
		{
			using type = spirv_cross::SPIRType::BaseType;
			uint32_t vecSize = spirType.vecsize;
			uint32_t columns = spirType.columns;

			ShaderDataType baseType = ShaderDataType::None;

			switch (spirType.basetype)
			{
				case type::Unknown: baseType = ShaderDataType::None; break;
				case type::Void: baseType = ShaderDataType::None; break;
				case type::Float: 
					baseType = ShaderDataType::Float;
					break;
				case type::Int: 
					baseType = ShaderDataType::Int;
					break;
				case type::Boolean: baseType = ShaderDataType::Bool; break;
				default: PE_CORE_WARN("Unsupported data type in shader: {0}", (int)spirType.basetype); baseType = ShaderDataType::None; break;
			}

			if (columns == 1 && vecSize > 1) {
				// Vector
				switch (baseType)
				{
				case ShaderDataType::Float:
					if (vecSize == 2) { return ShaderDataType::Float2; }
					if (vecSize == 3) { return ShaderDataType::Float3; }
					if (vecSize == 4) { return ShaderDataType::Float4; }
					break;
				case ShaderDataType::Int:
					if (vecSize == 2) { return ShaderDataType::Int2; }
					if (vecSize == 3) { return ShaderDataType::Int3; }
					if (vecSize == 4) { return ShaderDataType::Int4; }
					PE_CORE_WARN("Unsupported vector type in shader: baseType = {0}, vecSize = {1}", ShaderDataTypeToString(baseType).c_str(), vecSize);
					break;
				}
			}
			else if (columns > 1 && baseType == ShaderDataType::Float)
			{
				// Matrix
				if (columns == 3 && vecSize == 3) { return ShaderDataType::Mat3; }
				if (columns == 4 && vecSize == 4) { return ShaderDataType::Mat4; }
				PE_CORE_WARN("Unsupported matrix type in shader: baseType = {0}, columns = {1}, rows = {2}", ShaderDataTypeToString(baseType).c_str(), columns, vecSize);
				return ShaderDataType::None;
			}

			return baseType;
		}

		static void AddSpirTypeToUBOSpecRecursive(const spirv_cross::Compiler& compiler, spirv_cross::SPIRType memberType, const std::string& memberName, Ref<UBOShaderParameterTypeSpecification> uboSpec)
		{
			int arrayDimensions = memberType.array.size();
			if (arrayDimensions > 0) {
				for (int dimension = 0; dimension < arrayDimensions; dimension++)
				{
					for (int x = 0; x < memberType.array[dimension]; x++) {
						int childCount = memberType.member_types.size();
						if (childCount > 0) {
							for (int child = 0; child < childCount; child++) {
								spirv_cross::TypeID childTypeID = memberType.member_types[child];
								spirv_cross::SPIRType childType = compiler.get_type(childTypeID);
								std::string indexedName = memberName + "[" + std::to_string(dimension) + "]" + "[" + std::to_string(x) + "]";
								const std::string childName = indexedName + "." + compiler.get_member_name(memberType.self, child);

								AddSpirTypeToUBOSpecRecursive(compiler, childType, childName, uboSpec);
							}
						}
						else {
							ShaderDataType dataType = OpenGLShaderUtils::SpirTypeToShaderDataType(memberType);
							std::string shaderDataTypeString = ShaderDataTypeToString(dataType);

							std::string indexedName = memberName + "[" + std::to_string(dimension) + "]" + "[" + std::to_string(x) + "]";
							PE_CORE_TRACE("       - {0}: {1} ({2})", 0, indexedName.c_str(), shaderDataTypeString.c_str());
							uboSpec->BufferLayout.emplace_back(indexedName, dataType);
						}
					}
				}
			}
			else {
				int childCount = memberType.member_types.size();
				if (childCount > 0) {
					for (int child = 0; child < childCount; child++) {
						spirv_cross::TypeID childTypeID = memberType.member_types[child];
						spirv_cross::SPIRType childType = compiler.get_type(childTypeID);
						const std::string childName = memberName + "." + compiler.get_member_name(memberType.self, child);

						AddSpirTypeToUBOSpecRecursive(compiler, childType, childName, uboSpec);
					}
				}
				else {
					ShaderDataType dataType = OpenGLShaderUtils::SpirTypeToShaderDataType(memberType);
					std::string shaderDataTypeString = ShaderDataTypeToString(dataType);

					PE_CORE_TRACE("       - {0}: {1} ({2})", 0, memberName.c_str(), shaderDataTypeString.c_str());
					uboSpec->BufferLayout.emplace_back(memberName, dataType);
				}
			}
		}
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc, RenderPipelineContext shaderContext) : m_Name(name), m_Filepath("null"), m_RendererID(0), m_ShaderContext(shaderContext)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(m_ShaderContext != RenderPipelineContext::Undefined, "Undefined shader context");

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		sources[GL_GEOMETRY_SHADER] = geometrySrc;

		CompileOrGetOpenGLBinaries(sources);
		CreateProgram();
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc, RenderPipelineContext shaderContext) : m_Name(name), m_Filepath("null"), m_RendererID(0), m_ShaderContext(shaderContext)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(m_ShaderContext != RenderPipelineContext::Undefined, "Undefined shader context");

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;

		CompileOrGetOpenGLBinaries(sources);
		CreateProgram();
	}

	OpenGLShader::OpenGLShader(const std::string& filepath) : m_Filepath(filepath), m_RendererID(0), m_ShaderContext(RenderPipelineContext::Undefined)
	{
		PE_PROFILE_FUNCTION();
		
		OpenGLShaderUtils::ValidateCacheDirectory();

		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);
		
		CompileOrGetOpenGLBinaries(shaderSources);
		CreateProgram();

		// Extract name from filepath
		size_t lastSlash = filepath.find_last_of("/\\");
		lastSlash = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
		size_t lastDot = filepath.rfind('.');
		size_t count = (lastDot == std::string::npos) ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);

		PE_CORE_ASSERT(m_ShaderContext != RenderPipelineContext::Undefined, "Undefined shader context");
	}

	OpenGLShader::~OpenGLShader()
	{
		PE_PROFILE_FUNCTION();
		glDeleteProgram(m_RendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filepath)
	{
		PE_PROFILE_FUNCTION();
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in) {
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1) {
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
				in.close();
			}
			else {
				PE_CORE_ERROR("Could not read from file '{0}'", filepath);
			}
		}
		else { PE_CORE_ERROR("Failed to open file '{0}'", filepath); }
		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		PE_PROFILE_FUNCTION();

		// Find context type
		const char* contextToken = "#context";
		size_t contextTokenLength = strlen(contextToken);
		size_t contextPos = source.find(contextToken, 0);
		PE_CORE_ASSERT(contextPos != std::string::npos, "Shader context required");
		size_t contextEol = source.find_first_of("\r\n", contextPos);
		PE_CORE_ASSERT(contextEol != std::string::npos, "Syntax error");
		size_t contextBegin = contextPos + contextTokenLength + 1;
		std::string contextType = source.substr(contextBegin, contextEol - contextBegin);
		m_ShaderContext = RenderPipelineContextFromString(contextType);

		// Extract shader sources
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);

		while (pos != std::string::npos) {
			size_t eol = source.find_first_of("\r\n", pos);
			PE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			PE_CORE_ASSERT(OpenGLShaderUtils::ShaderTypeFromString(type), "Invalid shader type");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol);
			PE_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos);

			shaderSources[OpenGLShaderUtils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	void OpenGLShader::CompileOrGetOpenGLBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		PE_PROFILE_FUNCTION();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		options.SetGenerateDebugInfo();

		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		const bool optimize = true;
		if (optimize) { options.SetOptimizationLevel(shaderc_optimization_level_performance); }

		std::filesystem::path cacheDirectory = OpenGLShaderUtils::GetCacheDirectory();
		
		auto& shaderData = m_OpenGLSPIRV;
		shaderData.clear();

		for (auto&& [stage, source] : shaderSources) {
			std::filesystem::path shaderFilepath = m_Filepath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilepath.filename().string() + OpenGLShaderUtils::GLShaderStageCachedOpenGLFileExtension(stage));

			// First check for cached SpirV shader
			std::ifstream in = std::ifstream(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open()) {
				in.seekg(0, std::ios::end);
				size_t size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else {
				// Compile and cache into SpirV
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, OpenGLShaderUtils::GLShaderStageToShaderC(stage), m_Filepath.c_str());
				if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
					PE_CORE_ERROR(module.GetErrorMessage());
					PE_CORE_ASSERT(false, "Error compiling OpenGL shader into OpenGL-SpirV cache");
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out = std::ofstream(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open()) {
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		//for (auto&& [stage, data] : shaderData) {
		//	Reflect(stage, data);
		//}
	}

	void OpenGLShader::CreateProgram()
	{
		GLuint program = glCreateProgram();
		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : m_OpenGLSPIRV) {
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);
		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			PE_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_Filepath, infoLog.data());

			glDeleteProgram(program);
			for (GLuint id : shaderIDs) {
				glDeleteShader(id);
			}
		}

		for (GLuint id : shaderIDs) {
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
	}

	void OpenGLShader::AsVectorType(ShaderDataType& type, const SpvReflectBlockVariable* member)
	{
		uint32_t component_count = member->numeric.vector.component_count;

		if (member->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
		{
			type = (ShaderDataType)(component_count + ((int)ShaderDataType::Float - 1)); // Float = 1, Float2 = 2, Float3 = 3, Float4 = 4
		}
		else if (member->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_INT)
		{
			type = (ShaderDataType)(component_count + ((int)ShaderDataType::Int - 1)); // Int = 7, Int2 = 8, Int3 = 9, Int4 = 10
		}
		else if (member->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_BOOL)
		{
			PE_CORE_WARN("Bool vector type not supported, using Int instead");
			type = (ShaderDataType)(component_count + ((int)ShaderDataType::Int - 1));
		}
	}

	void OpenGLShader::AsMatrixType(ShaderDataType& type, const SpvReflectBlockVariable* member)
	{
		uint32_t column_count = member->numeric.matrix.column_count;
		uint32_t row_count = member->numeric.matrix.row_count;

		bool four = (column_count == 4 && row_count == 4);
		if (!four && (column_count == 3 && row_count == 3))
		{
			PE_CORE_WARN("Unsupported matrix dimensions");
		}

		if (member->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
		{
			type = four ? ShaderDataType::Mat4 : ShaderDataType::Mat3;
		}
		else if (member->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_INT)
		{
			PE_CORE_WARN("Int matrix type not supported");
		}
		else if (member->type_description->type_flags & SPV_REFLECT_TYPE_FLAG_BOOL)
		{
			PE_CORE_WARN("Bool matrix type not supported");
		}
	}

	void OpenGLShader::OverrideArrayOpType(SpvOp& op, SpvReflectTypeFlags flags)
	{
		if (op == SpvOpTypeArray)
		{
			if (flags & SPV_REFLECT_TYPE_FLAG_FLOAT) { op = SpvOpTypeFloat; }
			if (flags & SPV_REFLECT_TYPE_FLAG_INT) { op = SpvOpTypeInt; }
			if (flags & SPV_REFLECT_TYPE_FLAG_BOOL) { op = SpvOpTypeBool; }
			if (flags & SPV_REFLECT_TYPE_FLAG_MATRIX) { op = SpvOpTypeMatrix; }
			if (flags & SPV_REFLECT_TYPE_FLAG_VECTOR) { op = SpvOpTypeVector; }
			if (flags & SPV_REFLECT_TYPE_FLAG_STRUCT) { op = SpvOpTypeStruct; }
		}
	}

	void OpenGLShader::ReflectBlockVariableRecursive(spv_reflect::ShaderModule& reflection, SpvReflectBlockVariable* member, const std::string& parentName)
	{
		bool isStruct = false;
		ShaderDataType type = ShaderDataType::None;

		// Op type
		SpvOp op = member->type_description->op;
		SpvReflectTypeFlags flags = member->type_description->type_flags;
		OverrideArrayOpType(op, flags);

		const char* struct_member_name = member->type_description->struct_member_name;
		switch (op)
		{
		case SpvOpTypeFloat:
			type = ShaderDataType::Float;
			break;
		case SpvOpTypeInt:
			type = ShaderDataType::Int;
			break;
		case SpvOpTypeBool:
			type = ShaderDataType::Bool;
			break;
		case SpvOpTypeMatrix:
			AsMatrixType(type, member);
			break;
		case SpvOpTypeVector:
			AsVectorType(type, member);
			break;
		case SpvOpTypeStruct:
			isStruct = true;
			break;
		case SpvOpTypeArray:
			isStruct = true;
			break;
		}

		uint32_t arrayDims = member->array.dims_count;
		uint32_t arraySize = 0;

		std::string name = member->name;
		if (parentName != "")
		{
			name = parentName + "." + name;
		}

		if (arrayDims > 0)
		{
			arraySize = member->array.dims[0];
			for (uint32_t x = 0; x < arrayDims; x++)
			{
				for (uint32_t y = 0; y < arraySize; y++)
				{
					std::string indexedName = name + std::string("[" + std::to_string(x) + "][" + std::to_string(y) + "]");
					if (isStruct)
					{
						uint32_t memberCount = member->member_count;
						for (uint32_t i = 0; i < memberCount; i++)
						{
							SpvReflectBlockVariable* child = &member->members[i];
							ReflectBlockVariableRecursive(reflection, child, indexedName);
						}
					}
					else { PE_CORE_TRACE("       - {0}: {1} ({2})", 0, indexedName.c_str(), ShaderDataTypeToString(type)); }
				}
			}
		}
		else
		{
			if (isStruct)
			{
				uint32_t memberCount = member->member_count;
				for (uint32_t i = 0; i < memberCount; i++)
				{
					SpvReflectBlockVariable* child = &member->members[i];
					ReflectBlockVariableRecursive(reflection, child, name);
				}
			}
			else { PE_CORE_TRACE("       - {0}: {1} ({2})", 0, name.c_str(), ShaderDataTypeToString(type)); }
		}
	}

	bool OpenGLShader::ReflectUBOs(spv_reflect::ShaderModule& reflection, std::string& error)
	{
		uint32_t count = 0;
		SpvReflectResult result;
		result = reflection.EnumerateDescriptorBindings(&count, nullptr);
		if (result != SPV_REFLECT_RESULT_SUCCESS)
		{
			error = "Failed to count descriptor bindings";
			return false;
		}

		std::vector<SpvReflectDescriptorBinding*> bindings(count);
		result = reflection.EnumerateDescriptorBindings(&count, bindings.data());
		if (result != SPV_REFLECT_RESULT_SUCCESS)
		{
			error = "Failed to enumerate descriptor bindings";
			return false;
		}

		// Write buffers
		for (uint32_t i = 0; i < count; i++)
		{
			const SpvReflectDescriptorBinding* binding = bindings[i];
			if (binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				PE_CORE_TRACE("  Uniform buffer: {0}", binding->name);
				PE_CORE_TRACE("    Binding: {0}", binding->binding);
				PE_CORE_TRACE("    Size   : {0}", binding->block.size);

				uint32_t member_count = binding->block.member_count;
				PE_CORE_TRACE("    Members: {0}", member_count);

				for (uint32_t i = 0; i < member_count; i++)
				{
					SpvReflectBlockVariable member = binding->block.members[i];
					ReflectBlockVariableRecursive(reflection, &member);
				}
			}
		}

		return true;
	}

	bool OpenGLShader::ReflectSamplers(spv_reflect::ShaderModule& reflection, std::string& error)
	{
		uint32_t count = 0;
		SpvReflectResult result;
		result = reflection.EnumerateDescriptorBindings(&count, nullptr);
		if (result != SPV_REFLECT_RESULT_SUCCESS)
		{
			error = "Failed to count descriptor bindings";
			return false;
		}

		std::vector<SpvReflectDescriptorBinding*> bindings(count);
		result = reflection.EnumerateDescriptorBindings(&count, bindings.data());
		if (result != SPV_REFLECT_RESULT_SUCCESS)
		{
			error = "Failed to enumerate descriptor bindings";
			return false;
		}

		// Write samplers
		for (uint32_t i = 0; i < count; i++)
		{
			const SpvReflectDescriptorBinding* binding = bindings[i];
			if (binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER)
			{
				PE_CORE_TRACE("         Sampler: {0}", binding->name);
				PE_CORE_TRACE("    Binding: {0}", binding->binding);

				switch (binding->user_type)
				{
				case SPV_REFLECT_USER_TYPE_TEXTURE_2D:
					PE_CORE_TRACE("    Type   : Sampler2D");
					break;
				case SPV_REFLECT_USER_TYPE_TEXTURE_CUBE:
					PE_CORE_TRACE("    Type   : SamplerCube");
					break;
				case SPV_REFLECT_USER_TYPE_TEXTURE_2D_ARRAY:
					PE_CORE_TRACE("    Type   : Sampler2DArray");
					break;
				case SPV_REFLECT_USER_TYPE_TEXTURE_CUBE_ARRAY:
					PE_CORE_TRACE("    Type   : SamplerCubeArray");
					break;
				default:
					PE_CORE_TRACE("    Type   : Unknown");
				}
				uint32_t dims = binding->array.dims_count;
				uint32_t arraySize = binding->array.dims[0];

				if (dims > 0)
				{
					PE_CORE_TRACE(" Array Dims: {0}", dims);
					PE_CORE_TRACE(" Array Size: {0}", arraySize);
				}
			}
		}

		return true;
	}

	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		// Generate reflection data for a shader

		size_t spirv_nbytes = shaderData.size() * sizeof(uint32_t);
		spv_reflect::ShaderModule reflection = spv_reflect::ShaderModule(spirv_nbytes, shaderData.data());

		if (reflection.GetResult() != SPV_REFLECT_RESULT_SUCCESS)
		{
			PE_CORE_ERROR("Error reflecting SPIRV data at source path '{0}'", m_Filepath.c_str());
			return;
		}

		PE_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", OpenGLShaderUtils::GLShaderStageToString(stage), m_Filepath);

		// Get uniform buffers
		// -------------------
		std::string error = "";
		bool success = ReflectUBOs(reflection, error);
		if (!success)
		{
			PE_CORE_ERROR("Error reflecting SPIRV data at source path '{0}'", m_Filepath.c_str());
			PE_CORE_ERROR("  - {0}", error.c_str());
		}


		// Get samplers
		// ------------
		error = "";
		success = ReflectSamplers(reflection, error);
		if (!success)
		{
			PE_CORE_ERROR("Error reflecting SPIRV data at source path '{0}'", m_Filepath.c_str());
			PE_CORE_ERROR("  - {0}", error.c_str());
		}
	}

	//void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	//{
	//	spirv_cross::Compiler compiler = spirv_cross::Compiler(shaderData);
	//	spirv_cross::ShaderResources resources = compiler.get_shader_resources();
	//
	//	PE_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", OpenGLShaderUtils::GLShaderStageToString(stage), m_Filepath);
	//	PE_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
	//	PE_CORE_TRACE("    {0} samplers", resources.sampled_images.size());
	//
	//	if (resources.uniform_buffers.size() > 0) {
	//		PE_CORE_TRACE("Uniform buffers:");
	//	}
	//	for (const auto& resource : resources.uniform_buffers) {
	//		const auto& bufferType = compiler.get_type(resource.base_type_id);
	//		uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
	//		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
	//		int memberCount = bufferType.member_types.size();
	//
	//		PE_CORE_TRACE("  {0}", resource.name.c_str());
	//		PE_CORE_TRACE("    Size = {0}", bufferSize);
	//		PE_CORE_TRACE("    Binding = {0}", binding);
	//		PE_CORE_TRACE("    Members = {0}", memberCount);
	//
	//		Ref<UBOShaderParameterTypeSpecification> uboSpec = CreateRef<UBOShaderParameterTypeSpecification>();
	//		uboSpec->Size = bufferSize;
	//		uboSpec->Binding = binding;
	//		uboSpec->Name = resource.name;
	//
	//		for (int i = 0; i < memberCount; i++) {
	//			spirv_cross::TypeID memberTypeID = bufferType.member_types[i];
	//			const std::string& memberName = compiler.get_member_name(resource.base_type_id, i);
	//			spirv_cross::SPIRType memberType = compiler.get_type(memberTypeID);
	//
	//			OpenGLShaderUtils::AddSpirTypeToUBOSpecRecursive(compiler, memberType, memberName, uboSpec);
	//		}
	//		m_ReflectionData.push_back(uboSpec);
	//	}
	//
	//	if (resources.sampled_images.size() > 0) {
	//		PE_CORE_TRACE("Image samplers:");
	//	}
	//	for (const auto& resource : resources.sampled_images) {
	//		const auto& samplerType = compiler.get_type(resource.base_type_id);
	//		uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
	//
	//		PE_CORE_TRACE("  {0}", resource.name.c_str());
	//		PE_CORE_TRACE("    Binding = {0}", binding);
	//
	//		spv::Dim dimensions = samplerType.image.dim;
	//		bool array = samplerType.image.arrayed;
	//		
	//		if (dimensions == spv::Dim::Dim2D)
	//		{
	//			if (array) {
	//				Ref<Sampler2DArrayShaderParameterTypeSpecification> arraySpec = CreateRef<Sampler2DArrayShaderParameterTypeSpecification>();
	//				arraySpec->Binding = binding;
	//				arraySpec->Name = resource.name;
	//
	//				m_ReflectionData.push_back(arraySpec);
	//
	//				PE_CORE_TRACE("    Type = Sampler2DArray");
	//			}
	//			else {
	//				Ref<Sampler2DShaderParameterTypeSpecification> imageSpec = CreateRef<Sampler2DShaderParameterTypeSpecification>();
	//				imageSpec->Binding = binding;
	//				imageSpec->Name = resource.name;
	//
	//				m_ReflectionData.push_back(imageSpec);
	//
	//				PE_CORE_TRACE("    Type = Sampler2D");
	//			}
	//		}
	//		else if (dimensions == spv::Dim::DimCube)
	//		{
	//			Ref<SamplerCubeShaderParameterTypeSpecification> cubeSpec = CreateRef<SamplerCubeShaderParameterTypeSpecification>();
	//			cubeSpec->Binding = binding;
	//			cubeSpec->Name = resource.name;
	//			m_ReflectionData.push_back(cubeSpec);
	//			PE_CORE_TRACE("    Type = SamplerCube");
	//		}
	//		else
	//		{
	//			PE_CORE_WARN("Unsupported sampler dimensions");
	//		}
	//
	//		const spirv_cross::SPIRType& spirType = compiler.get_type(resource.type_id);
	//		if (spirType.array.size() > 0) {
	//			PE_CORE_TRACE("    Array dimensions = {0}", spirType.array.size());
	//			PE_CORE_TRACE("    Array size = {0}", spirType.array[0]);
	//		}
	//	}
	//}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::SetUniformInt(const std::string& name, const int value)
	{
		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetUniformIntArray(const std::string& name, const int* values, uint32_t count)
	{
		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetUniformFloat(const std::string& name, const float value)
	{
		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		UploadUniformMat3(name, matrix);
	}

	void OpenGLShader::SetUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		UploadUniformMat4(name, matrix);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, const int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, const int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, const float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}