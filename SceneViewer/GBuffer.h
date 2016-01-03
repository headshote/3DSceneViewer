#pragma once

#include "FrameBuffer.h"

namespace framebuffers
{
	class GBuffer : public FrameBuffer
	{
	public:
		GBuffer(GLuint scrWdht, GLuint scrHght);
		~GBuffer();

	protected:
		virtual void renderToQuad(GLuint postProcessingShader, GLuint blurShader, GLuint fullScreenQuad, GLuint renderingQuad);

	private:
		//no copy assignement and construction
		GBuffer(const GBuffer&);
		GBuffer& operator=(const GBuffer&);

	};

}