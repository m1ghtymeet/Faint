#pragma once
/*
** ShaderGraph **

Single-file example of a simple Shader Graph implemented with ImGui + imgui-node-editor + OpenGL.
This demonstrates:
- Basic data model: Node, Pin, Link, Graph
- Simple node types (ConstFloat, Add, TextureSample, Output)
- Graph -> GLSL fragment shader compilation (topological ordering + snippet composition)
- ImGui UI using imgui-node-editor (aka ax::NodeEditor)
- Preview render to an FBO and display in ImGui


NOTES & DEPENDENCIES
- GLFW, GLAD (or another loader), ImGui, imgui-node-editor (https://github.com/thedmd/imgui-node-editor)
- stb_image for texture loading (optional)
- C++17
- This is a demonstration / starting point. Extend node types and robust error handling as needed.


Compile (example):
g++ -std=c++17 ShaderGraph_ImGui_OpenGL.cpp -lglfw -ldl -lGL -lX11 -lpthread -o shader_graph
(Adjust includes and link libraries for your platform. Use GLAD loader initialisation before OpenGL calls.)
*/

#include "../UI/Panels/PanelWindow.h"

namespace Moon::Editor {
	class ShaderGraph : public PanelWindow {
	public:
		ShaderGraph(
			const std::string& p_name,
			bool p_opened,
			const PanelWindowSettings& p_windowSettings
		);

	protected:
		void _Draw_Impl() override;
	};
}