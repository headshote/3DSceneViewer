#pragma once

#include "ShadowMap.h"

namespace shadows
{
	class PointShadowMap : public ShadowMap
	{
	public:
		PointShadowMap(const GLuint width, const GLuint height);

		GLfloat getFarPLane();

	protected:
		//no copy assignement and construction
		PointShadowMap(const PointShadowMap&);
		PointShadowMap& operator=(const PointShadowMap&);

		GLfloat farPlane;

		virtual glm::mat4 setShaderUniforms(const GLuint shadowShader, const GLuint shadowBatchShader, const glm::vec3& lightSourcePosition);
		virtual void renderModels(const GLuint shadowShader, const GLuint shadowBatchShader,
			std::vector<models::Model>& theModels, std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>>& modelContexts);
	};
}
