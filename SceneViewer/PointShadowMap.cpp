#include "PointShadowMap.h"

using namespace shadows;

PointShadowMap::PointShadowMap(const GLuint width, const GLuint height) : ShadowMap(width, height)
{
	//frame buffer
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	GLuint depthCubemap;
	glGenTextures(1, &depthCubemap);

	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (GLuint i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
		shadowViewPrtWidth, shadowViewPrtHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//Normally we'd attach a single face of a cubemap texture to the framebuffer object and render the scene 6 times, 
	//each time switching the depth buffer target of the framebuffer to a different cubemap face. 
	//Since we're going to use a geometry shader that allows us to render to all faces in a single pass
	//we can directly attach the cubemap as a framebuffer's depth attachment using glFramebufferTexture: 
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shadowmapFBO = depthMapFBO;
	shadowmapTexture = depthCubemap;
}

GLfloat PointShadowMap::getFarPLane()
{
	return farPlane;
}

glm::mat4 PointShadowMap::setShaderUniforms(const GLuint shadowShader, const GLuint shadowBatchShader, const glm::vec3& lightSourcePosition)
{
	GLfloat aspect = (GLfloat)shadowViewPrtWidth / (GLfloat)shadowViewPrtHeight;
	GLfloat near = 0.05f;
	GLfloat far = 30.0f;
	glm::mat4 shadowProj = glm::perspective(90.0f, aspect, near, far);

	std::vector<glm::mat4> lightSpaceMatrices = {
		shadowProj * glm::lookAt(lightSourcePosition, lightSourcePosition + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
		shadowProj * glm::lookAt(lightSourcePosition, lightSourcePosition + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
		shadowProj * glm::lookAt(lightSourcePosition, lightSourcePosition + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),
		shadowProj * glm::lookAt(lightSourcePosition, lightSourcePosition + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)),
		shadowProj * glm::lookAt(lightSourcePosition, lightSourcePosition + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)),
		shadowProj * glm::lookAt(lightSourcePosition, lightSourcePosition + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0))
	};

	glUseProgram(shadowShader);
	for (GLuint i = 0; i < lightSpaceMatrices.size(); i++)
		glUniformMatrix4fv(glGetUniformLocation(shadowShader, (GLchar*)("lightSpaceTransforms[" + std::to_string(i) + "]").c_str()), 1,
		GL_FALSE,	//transpose?
		glm::value_ptr(lightSpaceMatrices[i]));
	glUniform3f(glGetUniformLocation(shadowShader, "lightPosition"), lightSourcePosition.x, lightSourcePosition.y, lightSourcePosition.z);
	glUniform1f(glGetUniformLocation(shadowShader, "far_plane"), far);

	glUseProgram(shadowBatchShader);
	for (GLuint i = 0; i < lightSpaceMatrices.size(); i++)
		glUniformMatrix4fv(glGetUniformLocation(shadowBatchShader, (GLchar*)("lightSpaceTransforms[" + std::to_string(i) + "]").c_str()), 1,
		GL_FALSE,	//transpose?
		glm::value_ptr(lightSpaceMatrices[i]));
	glUniform3f(glGetUniformLocation(shadowBatchShader, "lightPosition"), lightSourcePosition.x, lightSourcePosition.y, lightSourcePosition.z);
	glUniform1f(glGetUniformLocation(shadowBatchShader, "far_plane"), far);

	farPlane = far;

	return lightSpaceMatrices[0];
}

void PointShadowMap::renderModels(const GLuint shadowShader, const GLuint shadowBatchShader,
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
