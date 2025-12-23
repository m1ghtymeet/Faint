#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Moon::PostProcess::SSAO {
    std::vector<glm::vec3> ssaoKernel;
    std::vector<glm::vec3> ssaoNoise;

    static float RandomFloat(float a, float b) {
        return a + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (b - a)));
    }

    inline void Precompute() {
        ssaoKernel.clear();
        ssaoNoise.clear();
        ssaoKernel.reserve(64);
        ssaoNoise.reserve(16);

        for (int i = 0; i < 64; ++i) {
            glm::vec3 sample(
                RandomFloat(-1.0f, 1.0f),
                RandomFloat(-1.0f, 1.0f),
                RandomFloat(0.0f, 1.0f)
            );
            sample = glm::normalize(sample);
            sample *= RandomFloat(0.0f, 1.0f);
            float scale = float(i) / 64.0f;
            scale = glm::mix(0.1f, 1.0f, scale * scale);
            sample *= scale;
            ssaoKernel.push_back(sample);
        }

        // Noise texture (2D 4x4)
        for (int i = 0; i < 16; ++i) {
            glm::vec3 noise(
                RandomFloat(-1.0f, 1.0f),
                RandomFloat(-1.0f, 1.0f),
                0.0f
            );
            ssaoNoise.push_back(noise);
        }
    }
}
