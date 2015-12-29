#pragma once

#include "ModelRenderingContext.h"

namespace models
{
	class SingleCallContext : public ModelRenderingContext
	{
	public:
		SingleCallContext(glm::vec3& translation, glm::vec3& scale, glm::vec3& rotationAxis, GLfloat angle, GLboolean cull = true);

		virtual void applyContextStateToModel(Model& model);

		virtual void doRendering(Model& model, const GLuint shaderProgram, const GLuint batchShader);

	};

}