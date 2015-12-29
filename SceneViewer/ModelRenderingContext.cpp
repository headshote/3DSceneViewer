#include "ModelRenderingContext.h"

using namespace models;


ModelRenderingContext::ModelRenderingContext()
{

}

void ModelRenderingContext::appendTransformation(glm::mat4& transform)
{
	contextTransforms.push_back(transform);
}

glm::mat4 ModelRenderingContext::createTransform(glm::vec3& translation, glm::vec3& scale, glm::vec3& rotationAxis, GLfloat angle)
{
	glm::mat4 transform;

	transform = glm::translate(transform, translation);
	transform = glm::scale(transform, scale);
	transform = glm::rotate(transform, angle, rotationAxis);

	return transform;
}