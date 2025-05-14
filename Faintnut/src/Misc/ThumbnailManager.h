#pragma once
#include "Core/Base.h"
#include "Renderer/Types/Texture.h"
#include "Renderer/Types/Framebuffer2.h"


class ThumbnailManager {
private:
    std::unordered_map<std::string, Ref<Faint::Texture>> m_Thumbnails;

    Ref<Faint::Framebuffer2> m_Framebuffer;
    Ref<Faint::Framebuffer2> m_ShadedFramebuffer;
    const Faint::Vec2 m_ThumbnailSize = { 128, 128 };

    const uint32_t MAX_THUMBNAIL_PER_FRAME = 1;
    uint32_t m_ThumbnailGeneratedThisFrame = 0;

public:
    ThumbnailManager();
    ~ThumbnailManager() = default;

    static ThumbnailManager& Get();

    bool IsThumbnailLoaded(const std::string& path) const;
    Ref<Faint::Texture> GetThumbnail(const std::string& path);
    void MarkThumbnailAsDirty(const std::string& path);
    Ref<Faint::Texture> GenerateThumbnail(const std::string& path, Ref<Faint::Texture> texture);
    void OnEndFrame();
};
