#pragma once

#include "Common/Types.h"
#include "Core/Base.h"
#include "Debug/Log.h"

namespace Faint {

	static uint32_t ShaderDataTypeSize(ShaderDataType type) {
		switch (type) {
		case ShaderDataType::FLOAT:  return 4;
		case ShaderDataType::FLOAT2: return 4 * 2;
		case ShaderDataType::FLOAT3: return 4 * 3;
		case ShaderDataType::FLOAT4: return 4 * 4;
		case ShaderDataType::MAT3:   return 4 * 3 * 3;
		case ShaderDataType::MAT4:   return 4 * 4 * 4;
		case ShaderDataType::INT:    return 4;
		case ShaderDataType::INT2:   return 4 * 2;
		case ShaderDataType::INT3:   return 4 * 3;
		case ShaderDataType::INT4:   return 4 * 4;
		case ShaderDataType::BOOL:   return 1;
		}

		HZ_CORE_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}

	struct BufferElement {
		std::string name;
		ShaderDataType type;
		uint32_t offset;
		uint32_t size;
		bool normalized;

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: name(name), type(type), offset(0), size(ShaderDataTypeSize(type)), normalized(normalized) {
		}

		uint32_t GetComponentCount() const {
			switch (type) {
			case ShaderDataType::FLOAT:  return 1;
			case ShaderDataType::FLOAT2: return 2;
			case ShaderDataType::FLOAT3: return 3;
			case ShaderDataType::FLOAT4: return 4;
			case ShaderDataType::MAT3:   return 3 * 3;
			case ShaderDataType::MAT4:   return 4 * 4;
			case ShaderDataType::INT:    return 1;
			case ShaderDataType::INT2:   return 2;
			case ShaderDataType::INT3:   return 3;
			case ShaderDataType::INT4:   return 4;
			case ShaderDataType::BOOL:   return 1;
			}
			HZ_CORE_ASSERT(false, "Unknown ShaderDataType");
			return 0;
		}
	};

	class BufferLayout {
	public:
		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_elements(elements) {
			CalculateOffsetAndStride();
		}

		inline uint32_t GetStride() const { return m_stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_elements; }

		std::vector<BufferElement>::iterator begin() { return m_elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_elements.end(); }
	private:
		void CalculateOffsetAndStride() {
			uint32_t offset = 0;
			m_stride = 0;
			for (auto& element : m_elements) {
				element.offset = offset;
				offset += element.size;
				m_stride += element.size;
			}
		}

		std::vector<BufferElement> m_elements;
		uint32_t m_stride = 0;
	};

	class VertexBuffer {
	public:
		virtual ~VertexBuffer() {}

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;
	
		virtual const BufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;

		static Ref<VertexBuffer> Create(std::vector<Vertex>& vertices);
	};

	class IndexBuffer {
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static Ref<IndexBuffer> Create(std::vector<uint32_t>& indices);
	};
}