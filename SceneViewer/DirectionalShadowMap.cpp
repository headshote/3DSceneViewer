#include "DirectionalShadowMap.h"

using namespace shadows;

DirectionalShadowMap::DirectionalShadowMap(GLuint width, GLuint height) : ShadowMap(width, height)
{
	//frame buffer
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	//depth buffer texture
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		shadowViewPrtWidth, shadowViewPrtHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//attach the texture to the frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//We only need the depth information when rendering the scene from the light's perspective 
	//so there is no need for a color buffer. A framebuffer object however is not complete without a color buffer 
	//so we need to explicitly tell OpenGL we're not going to render any color data. 
	//We do this by setting both the read and draw buffer to GL_NONE
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shadowmapFBO = depthMapFBO;
	shadowmapTexture = depthMap;
}

glm::mat4 DirectionalShadowMap::setShaderUniforms(const GLuint shadowShader, const GLuint shadowBatchShader, const glm::vec3& lightSourcePosition)
{
	glm::mat4 projection = glm::ortho(-75.0f, 75.0f, -75.0f, 75.0f, 0.1f, 83.5f);

	glm::mat4 view = glm::lookAt(lightSourcePosition, glm::vec3(0.0f), glm::vec3(1.0f));

	glm::mat4 lightSpaceMatrix = projection * view;

	glUseProgram(shadowShader);
	glUniformMatrix4fv(glGetUniformLocation(shadowShader, "lightSpaceMatrix"), 1,
		GL_FALSE,	//transpose?
		glm::value_ptr(lightSpaceMatrix));

	glUseProgram(shadowBatchShader);
	glUniformMatrix4fv(glGetUniformLocation(shadowBatchShader, "lightSpaceMatrix"), 1,
		GL_FALSE,	//transpose?
		glm::value_ptr(lightSpaceMatrix));

	return lightSpaceMatrix;
}

void DirectionalShadowMap::renderModels(const GLuint shadowShader, const GLuint shadowBatchShader,
	std::vector<models::Model>& theModels, std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>>& modelContexts)
{
	//Rendering models
	for (GLuint i = 0; i < theModels.size(); ++i)
	{
		models::Model& model = theModels[i];

		std::vector<std::shared_ptr<models::ModelRenderingContext>> currentMContexts = modelContexts[model.getID()];

		for (GLuint j = 0; j < currentMContexts.size(); ++j)
			currentMContexts[j]->doRendering(model, shadowShader, shadowBatchShader, 0, 0);
	}
}
