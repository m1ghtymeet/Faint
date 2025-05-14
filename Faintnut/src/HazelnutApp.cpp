#include <Hazel.h>
#include <Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Faint {

    class Hazelnut : public Application {
    public:
        Hazelnut() : Application(ApplicationSpecification()) {
            //PushOverlay(new ImGuiLayer());
            PushLayer(new EditorLayer());
        }
    };

    Application* CreateApplication() {
        return new Hazelnut();
    }
}