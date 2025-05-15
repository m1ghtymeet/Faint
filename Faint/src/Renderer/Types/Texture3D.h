#include "Core/Base.h"
#include "Math/Math.h"
#include <glad/glad.h>

#include "AssetManagment/Serializable.h"

#include <string>

namespace Faint {

	class Texture3D : public ISerializable {
	public:
		Texture3D(const std::string& path);
		Texture3D(int width, int height, int d);

		void Bind(GLuint slot);
		void Unbind();

		GLuint GetID() {
			return m_ID;
		}

	private:
		GLuint m_ID;
	};
}