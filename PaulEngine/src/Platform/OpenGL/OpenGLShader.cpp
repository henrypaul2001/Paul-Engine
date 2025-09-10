#include "pepch.h"
#include "OpenGLShader.h"
#include "PaulEngine/Renderer/Resource/ShaderParameterType.h"

#include <glad/gl.h>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>

namespace PaulEngine {

	namespace OpenGLShaderUtils
	{
		struct UBOMember
		{
			std::string StrippedName;
			GLenum Type;
			GLint Offset;
			GLint ArraySize;
			GLint ArrayStride;
			GLint MatrixStride;
			GLboolean RowMajor;
		};

		static AssetType GLDataTypeToTextureAssetType(GLenum glType)
		{
			switch (glType)
			{
				case GL_SAMPLER_2D: return AssetType::Texture2D;
				case GL_SAMPLER_CUBE: return AssetType::TextureCubemap;
				case GL_SAMPLER_2D_ARRAY: return AssetType::Texture2DArray;
				case GL_SAMPLER_CUBE_MAP_ARRAY: return AssetType::TextureCubemapArray;
			}
			return AssetType::None;
		}

		static ShaderDataType GLDataTypeToShaderDataType(GLenum glType)
		{
			switch (glType)
			{
				case GL_FLOAT: return ShaderDataType::Float;
				case GL_FLOAT_VEC2: return ShaderDataType::Float2;
				case GL_FLOAT_VEC3: return ShaderDataType::Float3;
				case GL_FLOAT_VEC4: return ShaderDataType::Float4;
				case GL_INT: return ShaderDataType::Int;
				case GL_INT_VEC2: return ShaderDataType::Int2;
				case GL_INT_VEC3: return ShaderDataType::Int3;
				case GL_INT_VEC4: return ShaderDataType::Int4;
				case GL_BOOL: return ShaderDataType::Bool;
				case GL_FLOAT_MAT3: return ShaderDataType::Mat3;
				case GL_FLOAT_MAT4: return ShaderDataType::Mat4;
				case GL_SAMPLER_2D: return ShaderDataType::Sampler2DHandle;
				case GL_SAMPLER_2D_ARRAY: return ShaderDataType::Sampler2DArrayHandle;
				case GL_SAMPLER_CUBE: return ShaderDataType::SamplerCubeHandle;
				case GL_SAMPLER_CUBE_MAP_ARRAY: return ShaderDataType::SamplerCubeArrayHandle;
			}
			PE_CORE_WARN("Undefined shader data type glType:{0}", glType);
			return ShaderDataType::None;
		}

		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex") { return GL_VERTEX_SHADER; }
			if (type == "fragment" || type == "pixel") { return GL_FRAGMENT_SHADER; }
			if (type == "geometry") { return GL_GEOMETRY_SHADER; }
			PE_CORE_ASSERT(false, "Unknown shader type");
			return 0;
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

		static std::vector<std::string> FindTokenStrings(const std::string& source, const char* tokenName)
		{
			std::vector<std::string> results;

			size_t tokenLength = strlen(tokenName);
			size_t pos = source.find(tokenName, 0);

			while (pos != std::string::npos)
			{
				size_t eol = source.find_first_of("\r\n", pos);
				PE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
				size_t begin = pos + tokenLength + 1;
				std::string valueString = source.substr(begin, eol - begin);

				results.push_back(valueString);

				size_t nextLinePos = source.find_first_not_of("\r\n", eol);
				PE_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
				pos = source.find(tokenName, nextLinePos);
			}

			return results;
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
		std::vector<std::string> contextStrings = OpenGLShaderUtils::FindTokenStrings(source, "#context");
		PE_CORE_ASSERT(contextStrings.size() > 0, "Shader context required");
		m_ShaderContext = RenderPipelineContextFromString(contextStrings[0]);

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

		// Get defines
		std::vector<std::string> defineStrings = OpenGLShaderUtils::FindTokenStrings(source, "#define");
		for (const std::string& defineString : defineStrings)
		{
			size_t space = defineString.find_first_of(' ');
			std::string nameString = defineString.substr(0, space);
			std::string valueString = defineString.substr(space + 1, defineString.size() - space);
			m_Defines[nameString] = valueString;
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
			Reflect();

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
		PE_PROFILE_FUNCTION();
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

		Reflect();
	}

	void OpenGLShader::CacheProgramBinary(uint32_t program, const std::filesystem::path& cachePath)
	{
		PE_PROFILE_FUNCTION();
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
		PE_PROFILE_FUNCTION();
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

	void OpenGLShader::Reflect()
	{
		PE_PROFILE_FUNCTION();

		PE_CORE_TRACE("OpenGLShader::Reflect - '{0}'", m_Filepath);

		PE_CORE_TRACE("  Defines:");
		auto it = m_Defines.begin();
		while (it != m_Defines.end())
		{
			PE_CORE_TRACE("    {0}: {1}", it->first.c_str(), it->second.c_str());
			it++;
		}

		ReflectUBOs();
		ReflectSSBOs();
		ReflectSamplers();
	}

	void OpenGLShader::ReflectUBOs()
	{
		GLint numUBOs;
		glGetProgramInterfaceiv(m_RendererID, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &numUBOs);
		PE_CORE_TRACE("  UBO count  : {0}", (int)numUBOs);

		for (GLint i = 0; i < numUBOs; i++)
		{
			GLenum properties[4] = { GL_NAME_LENGTH, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE, GL_NUM_ACTIVE_VARIABLES };
			GLint results[4];
			glGetProgramResourceiv(m_RendererID, GL_UNIFORM_BLOCK, i, 4, properties, 4, nullptr, results);

			GLuint nameSize = (GLuint)results[0];
			int bindSlot = (int)results[1];
			int bufferSize = (int)results[2];
			int memberCount = (int)results[3];

			// UBO name
			std::string name = std::string((int)nameSize, '\0');
			glGetProgramResourceName(m_RendererID, GL_UNIFORM_BLOCK, i, nameSize, nullptr, name.data());
			if (!name.empty() && name.back() == '\0') { name.pop_back(); }

			PE_CORE_TRACE("  Uniform buffer: {0}", name.c_str());
			PE_CORE_TRACE("    Binding: {0}", bindSlot);
			PE_CORE_TRACE("    Size   : {0}", bufferSize);
			PE_CORE_TRACE("    Members: {0}", memberCount);

			Ref<UBOShaderParameterTypeSpecification> uboSpec = CreateRef<UBOShaderParameterTypeSpecification>();
			uboSpec->Size = bufferSize;
			uboSpec->Binding = bindSlot;
			uboSpec->Name = name;

			// Get indices of UBO's member uniforms
			GLenum memberProperty[1] = { GL_ACTIVE_VARIABLES };
			std::vector<GLint> memberIndices = std::vector<GLint>(memberCount, -1);
			glGetProgramResourceiv(m_RendererID, GL_UNIFORM_BLOCK, i, 1, memberProperty, memberCount, nullptr, memberIndices.data());

			// Collect members
			std::vector<OpenGLShaderUtils::UBOMember> members;
			members.reserve(memberCount);

			GLenum memberProperties[] = {
				GL_NAME_LENGTH,
				GL_TYPE,
				GL_OFFSET,
				GL_ARRAY_SIZE,
				GL_ARRAY_STRIDE,
				GL_MATRIX_STRIDE,
				GL_IS_ROW_MAJOR
			};

			for (GLint memberIndex : memberIndices)
			{
				GLint memberResults[7] = {};
				glGetProgramResourceiv(m_RendererID, GL_UNIFORM, memberIndex, 7, memberProperties, 7, nullptr, memberResults);

				GLuint memberNameSize = (GLuint)memberResults[0];
				std::string memberName = std::string((int)memberNameSize, '\0');
				glGetProgramResourceName(m_RendererID, GL_UNIFORM, memberIndex, memberNameSize, nullptr, memberName.data());

				if (!memberName.empty() && memberName.back() == '\0') { memberName.pop_back(); }

				size_t firstDot = memberName.find_first_of('.');
				std::string strippedName = memberName.substr(firstDot + 1, memberName.length() - firstDot);

				OpenGLShaderUtils::UBOMember member;
				member.StrippedName = strippedName;
				member.Type = (GLenum)memberResults[1];
				member.Offset = memberResults[2];
				member.ArraySize = memberResults[3];
				member.ArrayStride = memberResults[4];
				member.MatrixStride = memberResults[5];
				member.RowMajor = (GLboolean)memberResults[6];

				members.push_back(member);
			}

			// Sort members by memory layout
			std::sort(members.begin(), members.end(), [](const OpenGLShaderUtils::UBOMember& a, const OpenGLShaderUtils::UBOMember& b) {return a.Offset < b.Offset; });

			int memberIndex = 0;
			for (const OpenGLShaderUtils::UBOMember& member : members)
			{
				std::string strippedName = member.StrippedName;
				ShaderDataType type = OpenGLShaderUtils::GLDataTypeToShaderDataType(member.Type);

				PE_CORE_TRACE("       - {0}: ({1}) {2}", memberIndex++, ShaderDataTypeToString(type).c_str(), strippedName.c_str());
				uboSpec->BufferLayout.emplace_back(type, strippedName);

				for (int i = 1; i < member.ArraySize; i++)
				{
					strippedName.replace(strippedName.size() - 2, 1, std::to_string(i));
					PE_CORE_TRACE("       - {0}: ({1}) {2}", memberIndex++, ShaderDataTypeToString(type).c_str(), strippedName.c_str());
					uboSpec->BufferLayout.emplace_back(type, strippedName);
				}
			}

			m_ReflectionData.push_back(uboSpec);
		}
	}

	void OpenGLShader::ReflectSSBOs()
	{
		GLint numSSBOs;
		glGetProgramInterfaceiv(m_RendererID, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &numSSBOs);
		PE_CORE_TRACE("  SSBO count  : {0}", (int)numSSBOs);

		for (GLint i = 0; i < numSSBOs; i++)
		{
			GLenum properties[4] = { GL_NAME_LENGTH, GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE, GL_NUM_ACTIVE_VARIABLES };
			GLint results[4];
			glGetProgramResourceiv(m_RendererID, GL_SHADER_STORAGE_BLOCK, i, 4, properties, 4, nullptr, results);

			GLuint nameSize = (GLuint)results[0];
			int bindSlot = (int)results[1];
			int bufferSize = (int)results[2];
			int memberCount = (int)results[3];

			// UBO name
			std::string name = std::string((int)nameSize, '\0');
			glGetProgramResourceName(m_RendererID, GL_SHADER_STORAGE_BLOCK, i, nameSize, nullptr, name.data());
			if (!name.empty() && name.back() == '\0') { name.pop_back(); }

			PE_CORE_TRACE("  Storage buffer: {0}", name.c_str());
			PE_CORE_TRACE("    Binding: {0}", bindSlot);
			PE_CORE_TRACE("    Size   : {0}", bufferSize);
			PE_CORE_TRACE("    Members: {0}", memberCount);

			Ref<StorageBufferShaderParameterTypeSpecification> ssboSpec = CreateRef<StorageBufferShaderParameterTypeSpecification>();
			ssboSpec->Size = bufferSize;
			ssboSpec->Binding = bindSlot;
			ssboSpec->Name = name;

			// Get indices of UBO's member uniforms
			GLenum memberProperty[1] = { GL_ACTIVE_VARIABLES };
			std::vector<GLint> memberIndices = std::vector<GLint>(memberCount, -1);
			glGetProgramResourceiv(m_RendererID, GL_SHADER_STORAGE_BLOCK, i, 1, memberProperty, memberCount, nullptr, memberIndices.data());

			// Collect members
			std::vector<OpenGLShaderUtils::UBOMember> members;
			members.reserve(memberCount);

			GLenum memberProperties[] = {
				GL_NAME_LENGTH,
				GL_TYPE,
				GL_OFFSET,
				GL_ARRAY_SIZE,
				GL_ARRAY_STRIDE,
				GL_MATRIX_STRIDE,
				GL_IS_ROW_MAJOR,
				GL_TOP_LEVEL_ARRAY_SIZE,
				GL_TOP_LEVEL_ARRAY_STRIDE
			};

			bool containsDynamicArray = false;
			for (GLint memberIndex : memberIndices)
			{
				GLint memberResults[9] = {};
				glGetProgramResourceiv(m_RendererID, GL_BUFFER_VARIABLE, memberIndex, 9, memberProperties, 9, nullptr, memberResults);

				GLuint memberNameSize = (GLuint)memberResults[0];
				std::string memberName = std::string((int)memberNameSize, '\0');
				glGetProgramResourceName(m_RendererID, GL_BUFFER_VARIABLE, memberIndex, memberNameSize, nullptr, memberName.data());

				if (!memberName.empty() && memberName.back() == '\0') { memberName.pop_back(); }

				size_t firstDot = memberName.find_first_of('.');
				std::string strippedName = memberName.substr(firstDot + 1, memberName.length() - firstDot);

				OpenGLShaderUtils::UBOMember member;
				member.StrippedName = memberName;
				member.Type = (GLenum)memberResults[1];
				member.Offset = memberResults[2];
				member.ArraySize = memberResults[3];
				member.ArrayStride = memberResults[4];
				member.MatrixStride = memberResults[5];
				member.RowMajor = (GLboolean)memberResults[6];

				GLint topLevelArraySize = memberResults[7];
				GLint topLevelArrayStride = memberResults[8];

				if (topLevelArraySize > 1)
				{
					members.reserve(topLevelArraySize);
					std::string topLevelName = memberName.substr(0, memberName.find_first_of('['));
					for (GLint i = 0; i < topLevelArraySize; i++)
					{
						std::string arrayedName = topLevelName + "[" + std::to_string(i) + "]." + strippedName;

						OpenGLShaderUtils::UBOMember arrayedMember;
						arrayedMember.StrippedName = arrayedName;
						arrayedMember.Type = (GLenum)memberResults[1];
						arrayedMember.Offset = memberResults[2] + (i * topLevelArrayStride);
						arrayedMember.ArraySize = memberResults[3];
						arrayedMember.ArrayStride = memberResults[4];
						arrayedMember.MatrixStride = memberResults[5];
						arrayedMember.RowMajor = (GLboolean)memberResults[6];

						members.push_back(arrayedMember);
					}
				}
				else
				{
					if (topLevelArraySize == 0)
					{
						std::string topLevelName = memberName.substr(0, memberName.find_first_of('['));
						std::string arrayedName = topLevelName + "[DYNAMIC]." + strippedName;
						member.StrippedName = arrayedName;
						if (member.Offset < ssboSpec->DynamicArrayStart) { ssboSpec->DynamicArrayStart = member.Offset; }
						containsDynamicArray = true;
					}
					else if (member.ArraySize == 0)
					{
						std::string arrayedName = memberName.substr(0, memberName.find_first_of('[')) + "[DYNAMIC]";
						member.StrippedName = arrayedName;
						if (member.Offset < ssboSpec->DynamicArrayStart) { ssboSpec->DynamicArrayStart = member.Offset; }
						containsDynamicArray = true;
					}
					members.push_back(member);
				}
			}

			if (!containsDynamicArray) { ssboSpec->DynamicArrayStart = -1; }

			// Sort members by memory layout
			std::sort(members.begin(), members.end(), [](const OpenGLShaderUtils::UBOMember& a, const OpenGLShaderUtils::UBOMember& b) {return a.Offset < b.Offset; });

			size_t currentOffset = 0;
			int memberIndex = 0;
			for (const OpenGLShaderUtils::UBOMember& member : members)
			{
				std::string strippedName = member.StrippedName;
				ShaderDataType type = OpenGLShaderUtils::GLDataTypeToShaderDataType(member.Type);

				PE_CORE_TRACE("       - {0}: ({1}) {2}", memberIndex++, ShaderDataTypeToString(type).c_str(), strippedName.c_str());
				BufferElement e = BufferElement(type, strippedName);
				e.Offset = currentOffset;
				currentOffset += e.Size;

				ssboSpec->BufferLayout.push_back(e);

				for (int i = 1; i < member.ArraySize; i++)
				{
					strippedName.replace(strippedName.size() - 2, 1, std::to_string(i));
					PE_CORE_TRACE("       - {0}: ({1}) {2}", memberIndex++, ShaderDataTypeToString(type).c_str(), strippedName.c_str());
					BufferElement e = BufferElement(type, strippedName);
					e.Offset = currentOffset;
					currentOffset += e.Size;
					ssboSpec->BufferLayout.push_back(e);
				}
			}

			m_ReflectionData.push_back(ssboSpec);
			
			if (ssboSpec->Name.substr(0, 5) == "IMat_")
			{
				m_MaterialBufferSpecs.push_back(ssboSpec);

				size_t size = ssboSpec->Size;

				if (ssboSpec->DynamicArrayStart > -1)
				{
					size_t dynamicArrayElementSize = ssboSpec->Size - ssboSpec->DynamicArrayStart;
					size_t baseSize = ssboSpec->Size - dynamicArrayElementSize;

					size = baseSize + (dynamicArrayElementSize * MAX_INDIRECT_MATERIALS);
				};

				m_MaterialBuffers.push_back(ShaderStorageBuffer::Create(size, ssboSpec->Binding, StorageBufferMapping::None, true));
			}
		}
	}

	void OpenGLShader::ReflectSamplers()
	{
		GLint numUniforms;
		glGetProgramInterfaceiv(m_RendererID, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);
		PE_CORE_TRACE("  Uniform count : {0}", (int)numUniforms);

		// Not a fan of this. Due to uniform buffers, there could be hundreds of uniform variables and only a handful of uniform samplers
		// Iterate over all uniform variables in program to find sampler objects
		for (GLint i = 0; i < numUniforms; i++)
		{
			GLenum properties[2] = { GL_NAME_LENGTH, GL_LOCATION };
			GLint results[2];
			glGetProgramResourceiv(m_RendererID, GL_UNIFORM, i, 2, properties, 2, nullptr, results);

			GLuint nameSize = (GLuint)results[0];
			std::string name = std::string((GLuint)nameSize - 1, '\0');
			GLint arraySize;
			GLenum glType;
			glGetActiveUniform(m_RendererID, i, name.capacity() + 1, nullptr, &arraySize, &glType, name.data());

			AssetType textureAssetType = OpenGLShaderUtils::GLDataTypeToTextureAssetType(glType);
			if (Asset::IsTextureType(textureAssetType))
			{
				GLint location = results[1];
				GLint unit;
				glGetUniformiv(m_RendererID, location, &unit);

				int bindSlot = (int)unit;

				PE_CORE_TRACE("  Sampler       : {0}", name);
				PE_CORE_TRACE("    Binding: {0}", bindSlot);

				switch (textureAssetType)
				{
				case AssetType::Texture2D:
				{
					Ref<Sampler2DShaderParameterTypeSpecification> imageSpec = CreateRef<Sampler2DShaderParameterTypeSpecification>();
					imageSpec->Binding = bindSlot;
					imageSpec->Name = name;
					
					m_ReflectionData.push_back(imageSpec);
					PE_CORE_TRACE("    Type   : Sampler2D");
					break;
				}
				case AssetType::Texture2DArray:
				{
					Ref<Sampler2DArrayShaderParameterTypeSpecification> arraySpec = CreateRef<Sampler2DArrayShaderParameterTypeSpecification>();
					arraySpec->Binding = bindSlot;
					arraySpec->Name = name;
					
					m_ReflectionData.push_back(arraySpec);
					PE_CORE_TRACE("    Type   : Sampler2DArray");
					break;
				}
				case AssetType::TextureCubemap:
				{
					Ref<SamplerCubeShaderParameterTypeSpecification> cubeSpec = CreateRef<SamplerCubeShaderParameterTypeSpecification>();
					cubeSpec->Binding = bindSlot;
					cubeSpec->Name = name;
					
					m_ReflectionData.push_back(cubeSpec);
					PE_CORE_TRACE("    Type   : SamplerCube");
					break;
				}
				case AssetType::TextureCubemapArray:
				{
					Ref<SamplerCubeArrayShaderParameterTypeSpecification> cubeArraySpec = CreateRef<SamplerCubeArrayShaderParameterTypeSpecification>();
					cubeArraySpec->Binding = bindSlot;
					cubeArraySpec->Name = name;
					
					m_ReflectionData.push_back(cubeArraySpec);
					PE_CORE_TRACE("    Type   : SamplerCubeArray");
					break;
				}
				}
			}
		}
	}

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