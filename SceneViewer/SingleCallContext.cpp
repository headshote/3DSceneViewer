#include "SingleCallContext.h"

using namespace models;

SingleCallContext::SingleCallContext(glm::vec3& translation, glm::vec3& scale, glm::vec3& rotationAxis, GLfloat angle)
{
	appendTranslation(translation);
	appendScale(scale);
	appendRotation(rotationAxis, angle);
}

void SingleCallContext::applyContextStateToModel(Model& model)
{
	;
}

void SingleCallContext::doRendering(Model& model, const GLuint shaderProgram)
{
	model.setTranslation(cTranslations[0]);
	model.setScale(cScales[0]);
	model.setRotation(cRotationAxes[0], cRotations[0]);
	model.drawCall(shaderProgram);
}
