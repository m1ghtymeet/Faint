#pragma once

#include <memory>
//#include <sol/sol.hpp>

namespace Moon {

	class Entity;
}

namespace sol
{
	template <bool b>
	class basic_reference;
	using reference = basic_reference<false>;
	
	template <bool, typename>
	class basic_table_core;
	template <bool b>
	using table_core = basic_table_core<b, reference>;
	using table = table_core<false>;
}

namespace Moon::Scripting {

	class LuaScript {
	public:
		LuaScript(sol::table p_table);

		virtual ~LuaScript();

		void SetOwner(Entity& p_owner);

		bool IsValid() const;

		inline const std::unique_ptr<sol::table>& GetTable() const { return table; }

	private:
		//LuaScriptContext m_context;
		std::unique_ptr<sol::table> table;
	};
}