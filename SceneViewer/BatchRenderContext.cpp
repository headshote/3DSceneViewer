#include "BatchRenderContext.h"

using namespace models;

BatchRenderContext::BatchRenderContext(std::vector<glm::vec3>& translations, std::vector<glm::vec3>& scales, std::vector<glm::vec3>& rotationAxes, std::vector<GLfloat>& angles, GLboolean cull)
{
	if (translations.size() != scales.size() ||
		translations.size() != rotationAxes.size() || 
		translations.size() != angles.size())
		throw new std::exception("Arrays have diferent numbers of elements, wtf, nigguh?");

	for (GLuint i = 0; i < translations.size(); ++i)
	{
		appendTranslation(translations[i]);
		appendScale(scales[i]);
		appendRotation(rotationAxes[i], angles[i]);
	}

	cullFaces = cull;
}

void BatchRenderContext::applyContextStateToModel(Model& model)
{
	for (GLuint i = 0; i < cTranslations.size(); ++i)
	{
		model.setTranslation(cTranslations[i]);
		model.setScale(cScales[i]);
		model.setRotation(cRotationAxes[i], cRotations[i]);

		model.scheduleRendering();
	}
	model.flushScheduledInstances();
}

void BatchRenderContext::doRendering(Model& model, const GLuint shaderProgram, const GLuint batchShader)
{
	if (cullFaces)
		glEnable(GL_CULL_FACE);	//3d meshes, backface cull
	else
		glDisable(GL_CULL_FACE);	//2d quads, no cull

	model.batchRenderScheduledInstances(batchShader);
}
