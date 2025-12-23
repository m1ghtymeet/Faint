#pragma once

#include "../UI/Panels/AViewControllable.h"

class AssetView : public AViewControllable {
public:
	/**
	* Constructor
	* @param p_title
	* @param p_opened
	* @param p_windowSettings
	*/
	AssetView(const std::string& p_title = "Asset View", bool p_opened = true, const PanelWindowSettings& p_windowSettings = PanelWindowSettings());
	
	virtual Moon::Scene* GetScene();

	void SetModel(Moon::Model& p_model);

private:
	void _Draw_Impl() override;

	Moon::Entity* m_assetEntity;
	Moon::MeshRendererComponent* m_modelRenderer;
	Moon::Scene m_scene;
};