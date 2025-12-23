#pragma once

#include <AssetManagment/AssetManager.h>
#include <AssetManagment/Loader/TextureLoader.h>
#include <map>

namespace Moon::AssetManagment {

	class TextureManager : public AAssetManager<Assets::Texture> {
	public:
		/**
		* Create the resource identified by the given path
		* @param p_path
		*/
		virtual Assets::Texture* CreateResource(const std::string& p_path) override;

		/**
		* Destroy the given resource
		* @param p_resource
		*/
		virtual void DestroyResource(Assets::Texture* p_resource) override;

		/**
		* Reload the given resource
		* @param p_resource
		* @param p_path
		*/
		virtual void ReloadResource(Assets::Texture* p_resource, const std::string& p_path) override;
	};
}