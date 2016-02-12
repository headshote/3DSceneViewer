#pragma once

#include <vector>

#include "ModelRenderingContext.h"

namespace models
{
	class BatchRenderContext : public ModelRenderingContext
	{
	public:
		BatchRenderContext();
		BatchRenderContext(std::vector<glm::vec3>& translations, std::vector<glm::vec3>& scales, std::vector<glm::vec3>& rotationAxes, std::vector<GLfloat>& angles, GLboolean cull = true);

		virtual void applyContextStateToModel(Model& model);

		virtual void doRendering(Model& model, const GLuint shaderProgram, const GLuint batchShader, const GLuint outlineShader, const GLuint batchOutlineShader);

	};
}
