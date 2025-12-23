#pragma once

#include <optional>
#include <Renderer/Enums/BufferType.h>
#include <Renderer/Enums/AccessSpecified.h>

namespace Moon::Rendering {
	struct BufferMemoryRange {
		uint64_t offset;
		uint64_t size;
	};

	template<class BufferContext>
	class TBuffer {
	public:
		TBuffer(BufferType p_type);
		~TBuffer();

		uint64_t Allocate(uint64_t p_size, EAccessSpecified p_usage = EAccessSpecified::STATIC_DRAW);

		void Upload(const void* p_data, std::optional<BufferMemoryRange> p_range = std::nullopt);

		bool IsValid() const;

		bool IsEmpty() const;

		uint64_t GetSize() const;

		void Bind(std::optional<uint32_t> p_index = std::nullopt) const;

		void Unbind() const;

		uint32_t GetID() const;

	protected:
		BufferContext m_buffer;
	};
}