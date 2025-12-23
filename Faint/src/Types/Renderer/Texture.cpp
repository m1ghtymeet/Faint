#include "Texture.h"
#include <Debug/Assertion.h>

Moon::Assets::Texture::Texture(const std::string p_path, std::unique_ptr<Moon::Rendering::Texture>&& p_texture) :
    path(p_path) {
    SetTexture(std::move(p_texture));
}

void Moon::Assets::Texture::Bind(uint16_t slot) const {
        m_texture->Bind(slot);
}

void Moon::Assets::Texture::SetTexture(std::unique_ptr<Rendering::Texture>&& p_texture) {
    FT_CORE_ASSERT(m_texture != nullptr, "Cannot assign an invalid texture!");
    m_texture = std::move(p_texture);
}

Moon::Rendering::Texture& Moon::Assets::Texture::GetTexture() {
    FT_CORE_ASSERT(m_texture != nullptr, "Trying to access a null Texture");
    return *m_texture;
}
