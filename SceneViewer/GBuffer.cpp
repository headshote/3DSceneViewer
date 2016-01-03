#include "GBuffer.h"

using namespace framebuffers;

GBuffer::GBuffer(GLuint scrWdht, GLuint scrHght) : FrameBuffer(scrWdht, scrHght, std::vector<GLuint>(6))
{
	//1. position, 2. normal, 3. tangent, 4. bitangent, 5. diffuse+spec, 6. lightSpacePosition
	theFBO = makeFrameBuffer(colorBuffers, std::vector<GLboolean>{ false, false, false, false, true, true }, false, false);
}

GBuffer::~GBuffer()
{
}

void GBuffer::renderToQuad(GLuint postProcessingShader, GLuint blurShader, GLuint renderingQuad)
{
	//activate the default buffer (screen buffer)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);

	GLint oldPolygonMode;
	glGetIntegerv(GL_POLYGON_MODE, &oldPolygonMode);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Render the quad with the frame buffer texture on it
	glUseProgram(postProcessingShader);

	glUniform1i(glGetUniformLocation(postProcessingShader, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(postProcessingShader, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(postProcessingShader, "gTangent"), 2);
	glUniform1i(glGetUniformLocation(postProcessingShader, "gBitangent"), 3);
	glUniform1i(glGetUniformLocation(postProcessingShader, "gAlbedoSpec"), 4);
	glUniform1i(glGetUniformLocation(postProcessingShader, "glightSpacePosition"), 5);

	for (GLuint i = 0; i < colorBuffers.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
	}

	glBindVertexArray(renderingQuad);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//unbind objects
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//retrun to whatever polygon mode we used on the actual rendering
	glPolygonMode(GL_FRONT_AND_BACK, oldPolygonMode);
	glEnable(GL_DEPTH_TEST);

	//Blit the depth buffer from gBuffer to the default screen buffer, so that 3d objects could be drawn properly over the screen quad
	//after the deferred rendering, but not in a deferred way
	glBindFramebuffer(GL_READ_FRAMEBUFFER, theFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
	glBlitFramebuffer(0, 0, scrWidth, scrHeight, 0, 0, scrWidth, scrHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
