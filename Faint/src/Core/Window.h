#pragma once

#include "Core/Base.h"
#include "Core/Time.h"
#include "Math/Math.h"

struct GLFWwindow;

namespace Faint {

	class Scene;
	class Framebuffer2;

	class Window {
	public:
		Window();
		
		static Ref<Window> Get();

		GLFWwindow* GetWindow();

		bool ShouldClose();

		void Init();
		void Update(Time time);
		void FixedUpdate(Time time);
		void Draw();
		void EndDraw();

		//Ref<Framebuffer2> GetFrameBuffer() const;

		void SetScene(Ref<Scene> scene);
		Ref<Scene> GetScene();

		void SetSize(const Vec2& size);
		Vec2 GetSize() const;

		float GetWidth() { return width; }
		float GetHeight() { return height; }

		void SetPosition(const Vec2& position);
		void SetTitle(const std::string& title);
		void SetIcon(const std::string& path);
		void SetDecorated(bool enabled);
		void SetFullscreen(bool enabled);
		void Maximize();
		void ShowWindow(bool show);

		float viewportWidth = 0;
		float viewportHeight = 0;

	private:
		GLFWwindow* window;

		std::string title;
		uint32_t width;
		uint32_t height;

		Ref<Framebuffer2> framebuffer;
		Ref<Scene> scene;

		static Ref<Window> instance;

		void InitImgui();
	};
}