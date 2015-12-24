#pragma once

#include <vector>
#include <SOIL/SOIL.h>
#include "Renderable.h"

namespace renderables
{
	class SkyBox : public Renderable
	{
	public:
		explicit SkyBox(const std::vector<std::string>& textures_faces, GLboolean gammacorrection = false);
		~SkyBox();

		virtual GLuint getVAO();

	protected:
		virtual void render(const GLuint shaderprogram);
		virtual void batchRender(const GLuint shaderProgram, const GLuint numCalls);

	private:
		//no copy constructors and assignments
		SkyBox(const SkyBox&);
		SkyBox& operator=(const SkyBox&);

		void createCubeMap(const std::vector<std::string>& textures_faces);
		void generateCubeMapVAO(const std::vector<GLfloat>& skyboxVertices);

		GLuint cubemapID;	//cubemap texture id
		GLuint skyboxVAO;
		GLuint nrenderingElements;

		GLboolean useGammaCorrection;		
	};

}