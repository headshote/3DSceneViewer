#include "ShadowMap.h"

using namespace shadows;

ShadowMap::ShadowMap(GLuint width, GLuint height) : shadowViewPrtWidth(width), shadowViewPrtHeight(height)
{

}

ShadowMap::~ShadowMap()
{
	glDeleteFramebuffers(1, &shadowmapFBO);
	glDeleteTextures(1, &shadowmapTexture);
}

void ShadowMap::renderShadowMap(const GLuint shadowShader, const GLuint shadowBatchShader, const glm::vec3& lightSourcePosition,
	std::vector<models::Model>& models, std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>>& modelContexts)
{
	glViewport(0, 0, shadowViewPrtWidth, shadowViewPrtHeight);
	glCullFace(GL_FRONT);

	clearFrameBuffer(shadowmapFBO);

	lightSpaceMatrix = setShaderUniforms(shadowShader, shadowBatchShader, lightSourcePosition);
	renderModels(shadowShader, shadowBatchShader, models, modelContexts);
}

const GLuint ShadowMap::getTextureID()
{
	return shadowmapTexture;
}

const glm::mat4 ShadowMap::getLightSpaceMatrix()
{
	return lightSpaceMatrix;
}

/**
Sets up current frame buffer for rendering calls and clears it
*/
void ShadowMap::clearFrameBuffer(const GLuint FBO)
{
	//Clear current buffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}
