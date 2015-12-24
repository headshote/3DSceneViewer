#include "rendering.h"

namespace rendering
{
	GLboolean dotMode = false;

	GLboolean rearView = false;

	GLboolean highlightModels = false;
	
	GLboolean explodeMode = false;

	GLboolean renderNormals = false;

	GLboolean pointLightShadows = false;

	GLfloat hdrExposure = 1.0f;

	GLboolean deferredMode = false; 
	
	glm::vec3 spotLightColor(0.9f, 0.9f, 0.9f);

	GLuint screenShaderId = 0;
}