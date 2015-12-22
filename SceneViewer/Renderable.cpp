#include "Renderable.h"

using namespace renderables;

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
