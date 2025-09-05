#pragma once

namespace PaulEngine {
	enum class BufferUsage
	{
		None = 0,

		STATIC_DRAW,
		STATIC_READ,
		STATIC_COPY,

		DYNAMIC_DRAW,
		DYNAMIC_READ,
		DYNAMIC_COPY,

		STREAM_DRAW,
		STREAM_READ,
		STREAM_COPY
	};

	enum class ShaderDataType
	{
		None = 0,
		Float, Float2, Float3, Float4,
		Mat3, Mat4,
		Int, Int2, Int3, Int4,
		Bool,
		Sampler2DHandle, Sampler2DArrayHandle,
		SamplerCubeHandle, SamplerCubeArrayHandle
	};

	namespace BufferUtils
	{
		static const std::unordered_map<std::string, ShaderDataType> s_StringToShaderDataTypeMap = {
			{ "None",					ShaderDataType::None					},
			{ "Float",					ShaderDataType::Float					},
			{ "Float2",					ShaderDataType::Float2					},
			{ "Float3",					ShaderDataType::Float3					},
			{ "Float4",					ShaderDataType::Float4					},
			{ "Mat3",					ShaderDataType::Mat3					},
			{ "Mat4",					ShaderDataType::Mat4					},
			{ "Int",					ShaderDataType::Int						},
			{ "Int2",					ShaderDataType::Int2					},
			{ "Int3",					ShaderDataType::Int3					},
			{ "Int4",					ShaderDataType::Int4					},
			{ "Bool",					ShaderDataType::Bool					},
			{ "Sampler2DHandle",		ShaderDataType::Sampler2DHandle			},
			{ "Sampler2DArrayHandle",	ShaderDataType::Sampler2DArrayHandle	},
			{ "SamplerCubeHandle",		ShaderDataType::SamplerCubeHandle		},
			{ "SamplerCubeArrayHandle", ShaderDataType::SamplerCubeArrayHandle	}
		};
		static const std::unordered_map<ShaderDataType, std::string> s_ShaderDataTypeToStringMap = {
			{ ShaderDataType::None,						"None"					},
			{ ShaderDataType::Float,					"Float"					},
			{ ShaderDataType::Float2,					"Float2"				},
			{ ShaderDataType::Float3,					"Float3"				},
			{ ShaderDataType::Float4,					"Float4"				},
			{ ShaderDataType::Mat3,						"Mat3"					},
			{ ShaderDataType::Mat4,						"Mat4"					},
			{ ShaderDataType::Int,						"Int"					},
			{ ShaderDataType::Int2,						"Int2"					},
			{ ShaderDataType::Int3,						"Int3"					},
			{ ShaderDataType::Int4,						"Int4"					},
			{ ShaderDataType::Bool,						"Bool"					},
			{ ShaderDataType::Sampler2DHandle,			"Sampler2DHandle"		},
			{ ShaderDataType::Sampler2DArrayHandle,		"Sampler2DArrayHandle"	},
			{ ShaderDataType::SamplerCubeHandle,		"SamplerCubeHandle"		},
			{ ShaderDataType::SamplerCubeArrayHandle,	"SamplerCubeArrayHandle"}
		};
		static const std::unordered_map<ShaderDataType, size_t> s_ShaderDataTypeSizeMap = {
			{ ShaderDataType::None,						0	},
			{ ShaderDataType::Float,					4	},
			{ ShaderDataType::Float2,					8	},
			{ ShaderDataType::Float3,					12	},
			{ ShaderDataType::Float4,					16	},
			{ ShaderDataType::Mat3,						36	},
			{ ShaderDataType::Mat4,						64	},
			{ ShaderDataType::Int,						4	},
			{ ShaderDataType::Int2,						8	},
			{ ShaderDataType::Int3,						12	},
			{ ShaderDataType::Int4,						16	},
			{ ShaderDataType::Bool,						1	},
			{ ShaderDataType::Sampler2DHandle,			8	},
			{ ShaderDataType::Sampler2DArrayHandle,		8	},
			{ ShaderDataType::SamplerCubeHandle,		8	},
			{ ShaderDataType::SamplerCubeArrayHandle,	8	}
		};
		static const std::unordered_map<ShaderDataType, uint32_t> s_ShaderDataTypeComponentCountMap = {
			{ ShaderDataType::None,						0	},
			{ ShaderDataType::Float,					1	},
			{ ShaderDataType::Float2,					2	},
			{ ShaderDataType::Float3,					3	},
			{ ShaderDataType::Float4,					4	},
			{ ShaderDataType::Mat3,						9	},
			{ ShaderDataType::Mat4,						16	},
			{ ShaderDataType::Int,						1	},
			{ ShaderDataType::Int2,						2	},
			{ ShaderDataType::Int3,						3	},
			{ ShaderDataType::Int4,						4	},
			{ ShaderDataType::Bool,						1	},
			{ ShaderDataType::Sampler2DHandle,			2	},
			{ ShaderDataType::Sampler2DArrayHandle,		2	},
			{ ShaderDataType::SamplerCubeHandle,		2	},
			{ ShaderDataType::SamplerCubeArrayHandle,	2	}
		};
	}

	static std::string ShaderDataTypeToString(const ShaderDataType type)
	{
		auto it = BufferUtils::s_ShaderDataTypeToStringMap.find(type);
		if (it != BufferUtils::s_ShaderDataTypeToStringMap.end()) { return it->second; }
		PE_CORE_ASSERT(false, "Unknown ShaderDataType");
		return "Error";
	}

	static ShaderDataType StringToShaderDataType(const std::string& stringInput)
	{
		auto it = BufferUtils::s_StringToShaderDataTypeMap.find(stringInput);
		if (it != BufferUtils::s_StringToShaderDataTypeMap.end()) { return it->second; }
		PE_CORE_ASSERT(false, "Unknown ShaderDataType");
		return ShaderDataType::None;
	}

	static uint32_t ShaderDataTypeSize(const ShaderDataType type)
	{
		auto it = BufferUtils::s_ShaderDataTypeSizeMap.find(type);
		if (it != BufferUtils::s_ShaderDataTypeSizeMap.end()) { return it->second; }
		PE_CORE_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}

	static uint32_t GetComponentCount(const ShaderDataType type)
	{
		auto it = BufferUtils::s_ShaderDataTypeComponentCountMap.find(type);
		if (it != BufferUtils::s_ShaderDataTypeComponentCountMap.end()) { return it->second; }
		PE_CORE_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Size;
		uint32_t Offset;
		bool Normalized;

		BufferElement() = default;
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false) : Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) {}
	};

	class BufferLayout
	{
	public:
		BufferLayout() = default;
		BufferLayout(const std::initializer_list<BufferElement>& elements) : m_Elements(elements), m_Stride(0)
		{
			CalculateOffsetsAndStride();
		}

		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		inline uint32_t GetStride() const { return m_Stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements) {
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		virtual void SetData(const void* data, size_t size, size_t offset = 0) = 0;

		static Ref<VertexBuffer> Create(size_t size, BufferUsage usage = BufferUsage::DYNAMIC_DRAW);
		static Ref<VertexBuffer> Create(float* vertices, size_t size, BufferUsage usage = BufferUsage::STATIC_DRAW);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		inline virtual uint32_t GetCount() = 0;

		virtual void SetData(const uint32_t* indices, uint32_t count, size_t offset = 0) = 0;

		static Ref<IndexBuffer> Create(uint32_t count, BufferUsage usage = BufferUsage::DYNAMIC_DRAW);
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count, BufferUsage usage = BufferUsage::STATIC_DRAW);
	};
}