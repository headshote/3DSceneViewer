#ifndef RENDERING_H
#define RENDERING_H

#include <GL\glew.h>

namespace rendering
{
	extern GLboolean dotMode;

	extern GLboolean rearView;

	extern GLboolean highlightModels;

	extern GLboolean explodeMode;

	extern GLboolean renderNormals;

	//whether the first point light generates shadows
	extern GLboolean pointLightShadows;

	extern GLfloat hdrExposure;

	extern GLboolean deferredMode;
}

#endif	//RENDERING_H