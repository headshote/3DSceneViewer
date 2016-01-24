#include "MultisampledBlurFB.h"

using namespace framebuffers;

MultisampledBlurFB::MultisampledBlurFB(GLuint scrWdht, GLuint scrHght, GLuint nSamples) : FrameBuffer(scrWdht, scrHght, std::vector<GLuint>(2)), 
	pingPongFBOs(std::vector<GLuint>(2)),
	pingPongTextures(std::vector<GLuint>(2)), simpleColorBuffers(std::vector<GLuint>(2))
{
	//Let's create multisampled fbo for anti-aliasing
	GLuint FBO;
	glGenFramebuffers(1, &FBO);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	//Store here the list of color attachments, that we'll use for this particular framebuffer
	std::vector<GLuint> attachments;

	//Texture attachment for the frame buffer
	glGenTextures(colorBuffers.size(), &colorBuffers[0]);
	for (GLuint i = 0; i < colorBuffers.size(); i++)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorBuffers[i]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, nSamples, GL_RGB, scrWidth, scrHeight, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, colorBuffers[i], 0);
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	//renderbuffer object being attached (Usually used as a stencil, depth buffer)
	GLuint RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, nSamples, GL_DEPTH24_STENCIL8, scrWidth, scrHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	glDrawBuffers(colorBuffers.size(), &attachments[0]);

	//Correctness check
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::MULTISAMPLE_FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		throw new std::exception("ERROR::MULTISAMPLE_FRAMEBUFFER:: Framebuffer is not complete!");
	}

	//Bind back to the default framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	theFBO = FBO;

	//Now the simple fbo, to which multisampled is blitted, and which texture is rendered onto the screen quad
	simpleFBO = makeFrameBuffer(simpleColorBuffers, std::vector < GLboolean > {});

	//ping-pong fbos for blurring	
	std::vector<GLuint> pp1(1);
	pingPongFBOs[0] = makeFrameBuffer(pp1, std::vector < GLboolean > {}, true);
	pingPongTextures[0] = pp1[0];
	std::vector<GLuint> pp2(1);
	pingPongFBOs[1] = makeFrameBuffer(pp2, std::vector < GLboolean > {}, true);
	pingPongTextures[1] = pp2[0];
}

MultisampledBlurFB::~MultisampledBlurFB()
{
	glDeleteFramebuffers(pingPongFBOs.size(), &pingPongFBOs[0]);
	glDeleteTextures(pingPongTextures.size(), &pingPongTextures[0]);

	glDeleteFramebuffers(1, &simpleFBO);
	glDeleteTextures(simpleColorBuffers.size(), &simpleColorBuffers[0]);
}

void MultisampledBlurFB::renderToQuad(GLuint postProcessingShader, GLuint blurShader, GLuint fullScreenQuad, GLuint renderingQuad)
{
	blitMSampledScene(theFBO, simpleFBO);

	blurSceneBrightnessTextr(postProcessingShader, fullScreenQuad, simpleColorBuffers[1]);

	//activate the default buffer (screen buffer)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);

	//So that polygon mode for the quad is always fill
	GLint oldPolygonMode;
	glGetIntegerv(GL_POLYGON_MODE, &oldPolygonMode);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Render the quad with the frame buffer texture on it
	glUseProgram(postProcessingShader);
	glUniform1f(glGetUniformLocation(postProcessingShader, "exposure"), rendering::hdrExposure);

	glUniform1i(glGetUniformLocation(postProcessingShader, "screenTexture"), 0);
	glUniform1i(glGetUniformLocation(postProcessingShader, "brightnessTexture"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, simpleColorBuffers[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pingPongTextures[1]);

	glBindVertexArray(renderingQuad);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//unbind objects
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//retrun to whatever polygon mode we used on the actual rendering
	glPolygonMode(GL_FRONT_AND_BACK, oldPolygonMode);
	glEnable(GL_DEPTH_TEST);
}

/**
	Blits the colorbuffer from the multisampled FBO to the main FBO's color buffer (normal texture, that can be used for rendering)
*/
void MultisampledBlurFB::blitMSampledScene(GLuint multisampleFBO, GLuint sceneFBO)
{
	//Blit the multisampled frame buffer (where we have rendered the scene), to a non-multisampled frame buffer,
	//which texture will be mapped to a full-screen quad
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sceneFBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, scrWidth, scrHeight, 0, 0, scrWidth, scrHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glBlitFramebuffer(0, 0, scrWidth, scrHeight, 0, 0, scrWidth, scrHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

/**
	Using gauss blur, make multiple render calls, to render ping-pong buffers into a main fullscreen quad,
	exchanging their color textures, and blurring the brigtness buffer texture of the main rendering buffer
	Perfrom this before rendering the scene texture to the fullscreen quad
*/
void MultisampledBlurFB::blurSceneBrightnessTextr(GLuint blurShader, GLuint quadVao, GLuint brightnessTexture)
{
	//Blur the brigtness texture (with gauss blur)
	GLboolean horizontal = true, first_iteration = true;
	GLuint amount = 10;
	glUseProgram(blurShader);
	for (GLuint i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBOs[horizontal]);

		glUniform1i(glGetUniformLocation(blurShader, "horizontal"), horizontal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? brightnessTexture : pingPongTextures[!horizontal]);

		glBindVertexArray(quadVao);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		horizontal = !horizontal;
		first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
