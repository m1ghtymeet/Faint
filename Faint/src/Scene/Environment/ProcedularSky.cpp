#include "ProcedularSky.h"

#include "Renderer/ShadersManager.h"
#include <glad/glad.h>

namespace Faint {

	ProcedularSky::ProcedularSky()
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,


			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f
		};

		// setup plane VAO
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	void ProcedularSky::Draw(Matrix4 projection, Matrix4 view)
	{
		Shader skyShader = ShaderManager::GetShader("atmospheric_sky");
		skyShader.Bind();
		skyShader.SetFloat("SurfaceRadius", SurfaceRadius);
		skyShader.SetFloat("AtmosphereRadius", AtmosphereRadius);
		skyShader.SetFloat("SunIntensity", SunIntensity);
				 
		skyShader.SetMat4("Projection", projection);
		skyShader.SetMat4("View", view);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	Vec3 ProcedularSky::GetSunDirection()
	{
		return SunDirection;
	}
}
