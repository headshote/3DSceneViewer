#include "Renderable.h"

Renderable::Renderable()
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
