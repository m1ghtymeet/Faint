#pragma once

#include <functional>

namespace Faint {

	using ListenerID = uint64_t;

	template<class... ArgTypes>
	class Event
	{
	public:
		using Callback = std::function<void(ArgTypes...)>;
		ListenerID AddListener(Callback callback);
		bool RemoveListener(ListenerID listenerID);

		ListenerID operator+=(Callback callback);
		bool operator-=(ListenerID listenerID);
		
		void RemoveAllListeners();
		uint64_t GetListenerCount();
		void Invoke(ArgTypes... args);

	private:
		std::unordered_map<ListenerID, Callback> m_callbacks;
		ListenerID m_availableListenerID = 0;
	};
}

#include "Event.inl"