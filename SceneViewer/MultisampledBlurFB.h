#pragma once

#include "FrameBuffer.h"
#include "rendering.h"

namespace framebuffers
{
	class MultisampledBlurFB : public FrameBuffer
	{
	public:
		MultisampledBlurFB(GLuint scrWdht, GLuint scrHght, GLuint nSamples);

		~MultisampledBlurFB();

	protected:
		virtual void renderToQuad(GLuint postProcessingShader, GLuint blurShader, GLuint renderingQuad);

	private:
		//no copy assignement and construction
		MultisampledBlurFB(const MultisampledBlurFB&);
		MultisampledBlurFB& operator=(const MultisampledBlurFB&);

		void blitMSampledScene(GLuint multisampleFBO, GLuint sceneFBO);
		void blurSceneBrightnessTextr(GLuint blurShader, GLuint quadVao, GLuint brightnessTexture);

		std::vector<GLuint> pingPongFBOs;
		std::vector<GLuint> pingPongTextures;

		GLuint simpleFBO;
		std::vector<GLuint> simpleColorBuffers;
	};

}