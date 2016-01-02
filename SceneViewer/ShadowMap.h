#pragma once

#include <GL/glew.h>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Model.h"
#include "ModelRenderingContext.h"

namespace shadows
{
	class ShadowMap
	{
	public:
		virtual ~ShadowMap();

		void renderShadowMap(const GLuint shadowShader, const GLuint shadowBatchShader, const glm::vec3& lightSourcePosition,
			std::vector<models::Model>& theModels, std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>>& modelContexts);

		const GLuint getTextureID();
		const glm::mat4 getLightSpaceMatrix();

	protected:
		ShadowMap(const GLuint width, const GLuint height);

		void clearFrameBuffer(const GLuint FBO);

		virtual glm::mat4 setShaderUniforms(const GLuint shadowShader, const GLuint shadowBatchShader, const glm::vec3& lightSourcePosition) = 0;
		virtual void renderModels(const GLuint shadowShader, const GLuint shadowBatchShader,
			std::vector<models::Model>& theModels, std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>>& modelContexts) = 0;

		GLuint shadowViewPrtWidth;
		GLuint shadowViewPrtHeight;

		GLuint shadowmapFBO;
		GLuint shadowmapTexture;

		glm::mat4 lightSpaceMatrix;

	private:
		//no copy assignement and construction
		ShadowMap(const ShadowMap&);
		ShadowMap& operator=(const ShadowMap&);

	};

}