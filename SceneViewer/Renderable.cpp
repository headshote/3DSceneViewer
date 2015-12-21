#include "Renderable.h"

GLuint Renderable::getVAO()
{
	return VAO;
}

Renderable::Renderable()
{
}

/**
Render things
*/
void Renderable::drawCall(GLuint shederprogram)
{
	renderVAO(shederprogram, VAO, nRenderingElemts);
}

void Renderable::drawBatch(GLuint shederprogram, GLuint numDrawCalls)
{
	batchRenderVAO(shederprogram, VAO, nRenderingElemts, numDrawCalls, rendering::dotMode);
}
