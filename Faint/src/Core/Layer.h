#pragma once

#include "Core/Base.h"
#include "Core/Time.h"

namespace Faint {
	class Layer {
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Time time) {}
		virtual void OnImGuiRender() {}
		//virtual void OnEvent(Event& event) {}

		const std::string& GetName() const { return m_debugName; }
	protected:
		std::string m_debugName;
	};
}