#pragma once

#include <Renderer/Core/CompositeRenderer.h>

namespace Moon::PostProcess {
    
    class AEffect {
    public:
        AEffect(Rendering::CompositeRenderer& p_renderer);

        virtual void Draw(
            FrameBuffer& p_src,
            FrameBuffer& p_dst
        ) = 0;

    protected:
		Rendering::CompositeRenderer& m_renderer;
    };
}
