#include "FrameBuffer.h"

using namespace framebuffers;

FrameBuffer::FrameBuffer(GLuint scrWdht, GLuint scrHght, std::vector<GLuint> cbTextures) : colorBuffers(cbTextures), scrWidth(scrWdht), scrHeight(scrHght)
{
}


FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &theFBO);
	glDeleteTextures(colorBuffers.size(), &colorBuffers[0]);
}

void FrameBuffer::activateBuffer()
{
	glViewport(0, 0, scrWidth, scrHeight);
	glCullFace(GL_BACK);

	//Bind fbo as current
	glBindFramebuffer(GL_FRAMEBUFFER, theFBO);
	//Clear the current frame buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void FrameBuffer::renderColorBufferToQuad(GLuint postProcessingShader, GLuint blurShader, GLuint renderingQuad)
{
	renderToQuad(postProcessingShader, blurShader, renderingQuad);
}

/**
	Create a frame buffer object, basically a conteiner for a texture, that we're going to render our scene to.
	Later we could map this texture to a single quad, the size of a whole screen and do some post-processing in shaders at sonic speeed.
	Used as an actual output buffer, because you need to give shaders a non-multisampled texture for texelation (as in applying it to actual fragments).
	colorBufferParams : true - for alpha-containing color texture. false - for 16bit precision float (usually, data storage)
*/
GLuint FrameBuffer::makeFrameBuffer(std::vector<GLuint>& colorBufferTextures, std::vector<GLboolean>& colorBufferParams, GLboolean colorOnly, GLboolean linearApprox)
{
	GLuint FBO;
	glGenFramebuffers(1, &FBO);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	//Store here the list of color attachments, that we'll use for this particular framebuffer
	std::vector<GLuint> attachments;

	GLuint nColorBuffers = colorBufferTextures.size();

	//Texture attachment for the frame buffer
	glGenTextures(nColorBuffers, &colorBufferTextures[0]);
	for (GLuint i = 0; i < nColorBuffers; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBufferTextures[i]);
		if (i < colorBufferParams.size() && colorBufferParams[i])	//alpha, texture storage
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, scrWidth, scrHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		else	//noalpha, data buffer basically
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scrWidth, scrHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linearApprox ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linearApprox ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBufferTextures[i], 0);
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	if (!colorOnly)
	{
		//renderbuffer object being attached (Usually used as a stencil, depth buffer)
		GLuint RBO;
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, scrWidth, scrHeight);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	}

	//Give the list  of active color attachments for the current framebuffer
	glDrawBuffers(attachments.size(), &attachments[0]);

	//Correctness check
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		return 0;
	}

	//Bind back to the default framebuffer.
	//To make sure all rendering operations will have a visual impact on the main window 
	//we need to make the default framebuffer active again by binding to 0
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return FBO;
}
