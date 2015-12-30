#include "SingleCallContext.h"

using namespace models;

SingleCallContext::SingleCallContext(glm::vec3& translation, glm::vec3& scale, glm::vec3& rotationAxis, GLfloat angle, GLboolean cull)
{
	appendTranslation(translation);
	appendScale(scale);
	appendRotation(rotationAxis, angle);

	cullFaces = cull;
}

void SingleCallContext::applyContextStateToModel(Model& model)
{
	;
}

void SingleCallContext::doRendering(Model& model, const GLuint shaderProgram, const GLuint batchShader, const GLuint outlineShader, const GLuint batchOutlineShader)
{
	if ( cullFaces)
		glEnable(GL_CULL_FACE);	//3d meshes, backface cull
	else
		glDisable(GL_CULL_FACE);	//2d quads, no cull

	model.setTranslation(cTranslations[0]);
	model.setScale(cScales[0]);
	model.setRotation(cRotationAxes[0], cRotations[0]);

	if (rendering::highlightModels)
		model.drawOutlined(shaderProgram, outlineShader);
	else
		model.drawCall(shaderProgram);
}
