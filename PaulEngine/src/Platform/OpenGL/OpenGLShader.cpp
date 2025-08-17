#include "pepch.h"
#include "OpenGLShader.h"
#include "PaulEngine/Renderer/Resource/ShaderParameterType.h"

#include <glad/glad.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

#include <shaderc/shaderc.hpp>

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

		static void AsVectorType(ShaderDataType& type, const SpvReflectBlockVariable* member)
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

		static void AsMatrixType(ShaderDataType& type, const SpvReflectBlockVariable* member)
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

		static void OverrideArrayOpType(SpvOp& op, SpvReflectTypeFlags flags)
		{
			// The order of this is important as matrix types will be defined as both a
			// vector and a matrix, whereas a vector type will not be defined as a matrix
			if (op == SpvOpTypeArray)
			{
				if (flags & SPV_REFLECT_TYPE_FLAG_FLOAT) { op = SpvOpTypeFloat; }
				if (flags & SPV_REFLECT_TYPE_FLAG_INT) { op = SpvOpTypeInt; }
				if (flags & SPV_REFLECT_TYPE_FLAG_BOOL) { op = SpvOpTypeBool; }
				if (flags & SPV_REFLECT_TYPE_FLAG_VECTOR) { op = SpvOpTypeVector; }
				if (flags & SPV_REFLECT_TYPE_FLAG_MATRIX) { op = SpvOpTypeMatrix; }
				if (flags & SPV_REFLECT_TYPE_FLAG_STRUCT) { op = SpvOpTypeStruct; }
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

		if (!CompileOrGetOpenGLBinaries(sources))
		{
			CreateProgram();
		}
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc, RenderPipelineContext shaderContext) : m_Name(name), m_Filepath("null"), m_RendererID(0), m_ShaderContext(shaderContext)
	{
		PE_PROFILE_FUNCTION();
		PE_CORE_ASSERT(m_ShaderContext != RenderPipelineContext::Undefined, "Undefined shader context");

		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;

		if (!CompileOrGetOpenGLBinaries(sources))
		{
			CreateProgram();
		}
	}

	OpenGLShader::OpenGLShader(const std::string& filepath) : m_Filepath(filepath), m_RendererID(0), m_ShaderContext(RenderPipelineContext::Undefined)
	{
		PE_PROFILE_FUNCTION();
		
		OpenGLShaderUtils::ValidateCacheDirectory();

		std::string source = ReadFile(filepath);
		auto shaderSources = PreProcess(source);
		
		if (!CompileOrGetOpenGLBinaries(shaderSources))
		{
			CreateProgram();
		}

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

	bool OpenGLShader::CompileOrGetOpenGLBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		PE_PROFILE_FUNCTION();

		std::filesystem::path cacheDirectory = OpenGLShaderUtils::GetCacheDirectory();
		m_OpenGLBinaryIDMap.clear();

		// Try to load cached binary
		std::filesystem::path filepath = std::filesystem::path(m_Filepath);
		std::filesystem::path cachePath = cacheDirectory / (filepath.stem().string() + "GL.bin");
		if (LoadProgramBinary(cachePath))
		{
			PE_CORE_TRACE("Loaded cached shader program at path '{0}'", cachePath.string().c_str());

			// Load cached reflection data file

			return true;
		}

		// Compile each shader stage
		for (auto&& [stage, source] : shaderSources)
		{
			GLuint shader = glCreateShader(stage);
			const char* src = source.c_str();
			glShaderSource(shader, 1, &src, nullptr);
			glCompileShader(shader);

			GLint compiled;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
			if (!compiled)
			{
				GLint length;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
				std::vector<GLchar> infoLog = std::vector<GLchar>(length);
				glGetShaderInfoLog(shader, length, &length, infoLog.data());
				PE_CORE_ERROR("Shader compilation failed ({0}):\n{1}", m_Filepath, infoLog.data());
				glDeleteShader(shader);
			}

			m_OpenGLBinaryIDMap[stage] = shader;
		}

		return false;
	}

	void OpenGLShader::CreateProgram()
	{
		GLuint program = glCreateProgram();

		// Attach all compiled shaders
		for (auto&& [stage, shaderID] : m_OpenGLBinaryIDMap)
		{
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint linked;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			std::vector<GLchar> infoLog = std::vector<GLchar>(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			PE_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_Filepath, infoLog.data());

			glDeleteProgram(program);
			for (auto&& [stage, shaderID] : m_OpenGLBinaryIDMap)
			{
				glDeleteShader(shaderID);
			}
		}

		// Delete shaders after linking
		for (auto&& [stage, shaderID] : m_OpenGLBinaryIDMap)
		{
			glDetachShader(program, shaderID);
			glDeleteShader(shaderID);
		}

		m_OpenGLBinaryIDMap.clear();
		m_RendererID = program;

		GLint numFormats = 0;
		glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &numFormats);
		if (numFormats > 0)
		{
			std::filesystem::path filepath = std::filesystem::path(m_Filepath);
			std::filesystem::path cachePath = OpenGLShaderUtils::GetCacheDirectory() / std::filesystem::path(filepath.stem().string() + "GL.bin");
			CacheProgramBinary(program, cachePath);
		}
		else
		{
			PE_CORE_WARN("Driver does not support program binary reading");
		}
	}

	void OpenGLShader::CacheProgramBinary(uint32_t program, const std::filesystem::path& cachePath)
	{
		GLint length = 0;
		glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &length);
		if (length <= 0) { return; }

		std::vector<uint8_t> binary = std::vector<uint8_t>(length);
		GLenum format;
		glGetProgramBinary(program, length, nullptr, &format, binary.data());

		std::ofstream out = std::ofstream(cachePath, std::ios::binary);
		if (!out.is_open()) { 
			PE_CORE_ERROR("Error writing to path '{0}'", cachePath.string().c_str());
			return;
		}

		out.write(reinterpret_cast<const char*>(&format), sizeof(format));
		out.write(reinterpret_cast<const char*>(binary.data()), binary.size());
		out.close();
	}

	bool OpenGLShader::LoadProgramBinary(const std::filesystem::path& cachePath)
	{
		if (!std::filesystem::exists(cachePath)) { return false; }

		std::ifstream in = std::ifstream(cachePath, std::ios::binary);
		if (!in.is_open()) { return false; }
		
		GLenum format;
		in.read(reinterpret_cast<char*>(&format), sizeof(format));

		in.seekg(0, std::ios::end);
		size_t currentSize = in.tellg();
		size_t size = currentSize - sizeof(format);
		in.seekg(sizeof(format), std::ios::beg);

		std::vector<uint8_t> binary = std::vector<uint8_t>(size);
		in.read(reinterpret_cast<char*>(binary.data()), size);
		in.close();

		GLuint program = glCreateProgram();
		glProgramBinary(program, format, binary.data(), size);

		GLint linked;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			PE_CORE_ERROR("Failed to load cached shader program at path '{0}'", cachePath.string().c_str());
			glDeleteProgram(program);
			return false;
		}

		m_RendererID = program;
		return true;
	}

	/*
	void OpenGLShader::ReflectBlockVariableRecursive(spv_reflect::ShaderModule& reflection, SpvReflectBlockVariable* member, Ref<UBOShaderParameterTypeSpecification>& uboSpec, const std::string& parentName)
	{
		bool isStruct = false;
		ShaderDataType type = ShaderDataType::None;

		SpvOp op = member->type_description->op;
		SpvReflectTypeFlags flags = member->type_description->type_flags;
		OpenGLShaderUtils::OverrideArrayOpType(op, flags);

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
			OpenGLShaderUtils::AsMatrixType(type, member);
			break;
		case SpvOpTypeVector:
			OpenGLShaderUtils::AsVectorType(type, member);
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
							ReflectBlockVariableRecursive(reflection, child, uboSpec, indexedName);
						}
					}
					else {
						PE_CORE_TRACE("       - {0} ({1})", indexedName.c_str(), ShaderDataTypeToString(type));
						uboSpec->BufferLayout.emplace_back(indexedName, type);
					}
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
					ReflectBlockVariableRecursive(reflection, child, uboSpec, name);
				}
			}
			else {
				PE_CORE_TRACE("       - {0} ({1})", name.c_str(), ShaderDataTypeToString(type));
				uboSpec->BufferLayout.emplace_back(name, type);
			}
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
				const char* name = binding->type_description->type_name;
				uint32_t bindSlot = binding->binding;
				uint32_t bufferSize = binding->block.size;
				PE_CORE_TRACE("  Uniform buffer: {0}", name);
				PE_CORE_TRACE("    Binding: {0}", bindSlot);
				PE_CORE_TRACE("    Size   : {0}", bufferSize);

				uint32_t member_count = binding->block.member_count;
				PE_CORE_TRACE("    Members: {0}", member_count);

				Ref<UBOShaderParameterTypeSpecification> uboSpec = CreateRef<UBOShaderParameterTypeSpecification>();
				uboSpec->Name = name;
				uboSpec->Binding = bindSlot;
				uboSpec->Size = bufferSize;

				for (uint32_t i = 0; i < member_count; i++)
				{
					SpvReflectBlockVariable member = binding->block.members[i];
					ReflectBlockVariableRecursive(reflection, &member, uboSpec);
				}
				m_ReflectionData.push_back(uboSpec);
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
			if (binding->type_description->op == SpvOpTypeSampledImage)
			{
				const char* name = binding->name;
				uint32_t bindSlot = binding->binding;
				PE_CORE_TRACE("  Sampler       : {0}", name);
				PE_CORE_TRACE("    Binding: {0}", bindSlot);

				SpvDim dim = binding->image.dim;
				bool arrayed = (bool)binding->image.arrayed;

				switch (dim)
				{
				case SpvDim2D:
				{
					if (arrayed)
					{
						Ref<Sampler2DArrayShaderParameterTypeSpecification> arraySpec = CreateRef<Sampler2DArrayShaderParameterTypeSpecification>();
						arraySpec->Binding = bindSlot;
						arraySpec->Name = name;

						m_ReflectionData.push_back(arraySpec);
						PE_CORE_TRACE("    Type   : Sampler2DArray");
					}
					else
					{
						Ref<Sampler2DShaderParameterTypeSpecification> imageSpec = CreateRef<Sampler2DShaderParameterTypeSpecification>();
						imageSpec->Binding = bindSlot;
						imageSpec->Name = name;

						m_ReflectionData.push_back(imageSpec);
						PE_CORE_TRACE("    Type   : Sampler2D");
					}
					break;
				}
				case SpvDimCube:
				{
					if (arrayed)
					{
						Ref<SamplerCubeArrayShaderParameterTypeSpecification> cubeArraySpec = CreateRef<SamplerCubeArrayShaderParameterTypeSpecification>();
						cubeArraySpec->Binding = bindSlot;
						cubeArraySpec->Name = name;

						m_ReflectionData.push_back(cubeArraySpec);
						PE_CORE_TRACE("    Type   : SamplerCubeArray");
					}
					else
					{
						Ref<SamplerCubeShaderParameterTypeSpecification> cubeSpec = CreateRef<SamplerCubeShaderParameterTypeSpecification>();
						cubeSpec->Binding = bindSlot;
						cubeSpec->Name = name;

						m_ReflectionData.push_back(cubeSpec);
						PE_CORE_TRACE("    Type   : SamplerCube");
					}
					break;
				}
				default:
					PE_CORE_WARN("Unsupported sampler dimensions");
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
	*/

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