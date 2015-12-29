#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Model.h"

namespace models
{
	class Model;

	class ModelRenderingContext
	{
	public:
		virtual void doRendering(Model& model) = 0;

		virtual void applyContextStateToModel(Model& model) = 0;

	protected:
		ModelRenderingContext(/*glm::vec3& translation, glm::vec3& scale, glm::vec3& rotationAxis, GLfloat angle*/);

		void appendTransformation(glm::mat4& transform);

		std::vector<glm::mat4> contextTransforms;

		std::vector<GLuint> shaderIDs;
	};
}