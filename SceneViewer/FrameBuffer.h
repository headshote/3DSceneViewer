#pragma once

#include <GL/glew.h>
#include <vector>
#include <iostream>

namespace framebuffers
{
	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer();

		void activateBuffer();

		void renderColorBufferToQuad(GLuint postProcessingShader, GLuint blurShader, GLuint fullScreenQuad, GLuint renderingQuad);

	protected:
		FrameBuffer(GLuint scrWdht, GLuint scrHght, std::vector<GLuint> cbTextures);

		GLuint makeFrameBuffer(std::vector<GLuint>& colorBufferTextures, std::vector<GLboolean>& colorBufferParams, GLboolean colorOnly = false, GLboolean linearApprox = true);

		virtual void renderToQuad(GLuint postProcessingShader, GLuint blurShader, GLuint fullScreenQuad, GLuint renderingQuad) = 0;

		GLuint theFBO;
		std::vector<GLuint> colorBuffers;	//ids of textures, used as colorbuffers

		GLuint scrWidth;
		GLuint scrHeight;

	private:
		//no copy assignement and construction
		FrameBuffer(const FrameBuffer&);
		FrameBuffer& operator=(const FrameBuffer&);

	};

}