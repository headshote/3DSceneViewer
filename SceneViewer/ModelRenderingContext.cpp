#include "ModelRenderingContext.h"

using namespace models;

ModelRenderingContext::ModelRenderingContext() : cullFaces(true)
{

}

void ModelRenderingContext::appendTranslation(glm::vec3& translation)
{
	cTranslations.push_back(translation);
}

void ModelRenderingContext::appendScale(glm::vec3& scale)
{
	cScales.push_back(scale);
}

void ModelRenderingContext::appendRotation(glm::vec3& rotationAxis, GLfloat angle)
{
	cRotationAxes.push_back(rotationAxis);
	cRotations.push_back(angle);
}

void ModelRenderingContext::setTranslation(const glm::vec3 translation, GLuint transformId)
{
	cTranslations[transformId] = translation;
}

void ModelRenderingContext::setScale(const glm::vec3 scale, GLuint transformId)
{
	cScales[transformId] = scale;
}

void ModelRenderingContext::setRotation(const glm::vec3 rotationAxis, const GLfloat angle, GLuint transformId)
{
	cRotationAxes[transformId] = rotationAxis;
	cRotations[transformId] = angle;
}
