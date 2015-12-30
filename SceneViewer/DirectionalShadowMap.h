#pragma once

#include "ShadowMap.h"

namespace shadows
{
	class DirectionalShadowMap : public ShadowMap
	{
	public:
		DirectionalShadowMap(const GLuint width, const GLuint height);

	protected:
		//no copy assignement and construction
		DirectionalShadowMap(const DirectionalShadowMap&);
		DirectionalShadowMap& operator=(const DirectionalShadowMap&);

		virtual glm::mat4 setShaderUniforms(const GLuint shadowShader, const GLuint shadowBatchShader, const glm::vec3& lightSourcePosition);
		virtual void renderModels(const GLuint shadowShader, const GLuint shadowBatchShader,
			std::vector<models::Model>& theModels, std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>>& modelContexts);
	};
}
