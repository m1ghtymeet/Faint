#include "Core/Base.h"
#include "Math/Math.h"
#include <glad/glad.h>

#include "AssetManagment/Serializable.h"

#include <string>

namespace Faint {

	class ShadowMap {
	public:
		ShadowMap(int width, int height);

		void Bind(GLuint slot);
		void Unbind();

		GLuint GetID() { return m_ID; }
		GLuint GetTextureID() { return m_TextureID; }

	private:
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		GLuint m_ID;
		GLuint m_TextureID;
	};
}