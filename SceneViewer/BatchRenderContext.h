#pragma once

#include <vector>

#include "ModelRenderingContext.h"

namespace models
{
	class BatchRenderContext : public ModelRenderingContext
	{
	public:
		BatchRenderContext(std::vector<glm::vec3>& translations, std::vector<glm::vec3>& scales, std::vector<glm::vec3>& rotationAxes, std::vector<GLfloat>& angles);

		virtual void applyContextStateToModel(Model& model);

		virtual void doRendering(Model& model, const GLuint shaderProgram);

	};
}
