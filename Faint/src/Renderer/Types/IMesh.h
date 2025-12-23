#pragma once

namespace Moon::Rendering {
	class IMesh {
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual uint32_t GetVertexCount() const = 0;
		virtual uint32_t GetIndexCount() const = 0;
	};
}