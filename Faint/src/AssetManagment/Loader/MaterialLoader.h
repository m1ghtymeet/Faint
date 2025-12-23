#pragma once

#include "Renderer/Types/Material.h"

namespace Moon::Loaders {

	/**
	* Handle the creation of materials
	*/
	class MaterialLoader {
	public:
		/**
		* Disable constructor
		*/
		MaterialLoader() = delete;

		/**
		* Instantiate a material from a file
		* @param p_path
		*/
		static Rendering::Material* Create(const std::string& p_filepath, bool p_absolute = false);

		/**
		* Reload the material using the given file path
		* @param p_material
		* @param p_path
		*/
		static void Reload(Rendering::Material& p_material, const std::string& p_path);

		/**
		* Save the material to the given path
		* @param p_material
		* @param p_path
		*/
		static void Save(Rendering::Material& p_material, const std::string& p_path);
	};
}