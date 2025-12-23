
#include <sol/sol.hpp>
#include <Core/Window.h>
#include <Core/GlobalLocator.h>

#include <Types/Renderer/Model.h>
#include <Renderer/Types/Material.h>

void BindLuaCore(sol::state& p_state) {

	using namespace Moon;
	using namespace Moon::Rendering;

	p_state.create_named_table("Window",
		"GetWidth", []() {
			return FTSERVICE(Moon::Window).GetSize().first;
		},
		"GetHeight", []() {
			return FTSERVICE(Moon::Window).GetSize().second;
		},
		"SetFullscreen", [](bool fullscreen) {
			FTSERVICE(Moon::Window).SetFullscreen(fullscreen);
		}
	);

	p_state.new_usertype<Model>("Model",
		"GetMeshCount", &Model::GetMeshCount,
		"GetMeshIndices", &Model::GetMeshIndices,
		"path", sol::readonly_property(&Model::path)
	);

	p_state.new_usertype<Material>("Material",
		"SetFloat", [](Material& p_self, const std::string& name, float value) {
			p_self.SetProperty(name, value);
		},
		//"SetVec3", [](Material& p_self, const std::string& name, Vector3 value) {
		//	p_self.SetProperty(name, value);
		//}
		"SetBlendable", &Material::SetBlendable,
		"path", sol::readonly_property(&Material::path)
	);
}