#include "Texture3D.h"

namespace Faint {

	Texture3D::Texture3D(int width, int height, int depth) {
		
		glGenTextures(1, &m_ID);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, m_ID);
	
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, width, height, depth, 0, GL_RGBA, GL_FLOAT, NULL);
		glGenerateMipmap(GL_TEXTURE_3D);
		glBindImageTexture(0, m_ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
		
		
	}
}
