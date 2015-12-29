#include "ModelRenderingContext.h"

using namespace models;


ModelRenderingContext::ModelRenderingContext()
{

}

void ModelRenderingContext::appendTransformation(glm::mat4& transform)
{
	contextTransforms.push_back(transform);
}