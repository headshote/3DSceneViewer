#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Model.h"

namespace models
{
	class ModelRenderingContext
	{
	public:
		virtual void applyContextStateToModel(Model& model) = 0;

		virtual void doRendering(Model& model, const GLuint shaderProgram, const GLuint batchShader, const GLuint outlineShader, const GLuint batchOutlineShader) = 0;

		void setTranslation(const glm::vec3 translation, GLuint transformId);
		void setScale(const glm::vec3 scale, GLuint transformId);
		void setRotation(const glm::vec3 rotationAxis, const GLfloat angle, GLuint transformId);

		void appendTranslation(glm::vec3& translation);
		void appendScale(glm::vec3& scale);
		void appendRotation(glm::vec3& rotationAxis, GLfloat angle);

	protected:
		ModelRenderingContext();

		glm::mat4 createTransform(glm::vec3& translation, glm::vec3& scale, glm::vec3& rotationAxis, GLfloat angle);

		std::vector<glm::vec3> cTranslations;
		std::vector<glm::vec3> cScales;
		std::vector<glm::vec3> cRotationAxes;
		std::vector<GLfloat> cRotations;

		GLboolean cullFaces;
	};
}