#include "Renderable.h"

using namespace renderables;

Renderable::Renderable() : rMode(GL_TRIANGLES)
{
}

/**
Render things
*/
void Renderable::drawCall(const GLuint shederprogram)
{
	render(shederprogram);
}

void Renderable::drawBatch(const GLuint shederprogram, const GLuint numDrawCalls)
{
	batchRender(shederprogram, numDrawCalls);
}

void Renderable::setRendMode(GLenum mode)
{
	rMode = mode;
}
