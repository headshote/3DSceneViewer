#pragma once

#include <GL\glew.h>

namespace dataArrays
{
	//Square data
	GLfloat rectanglevertices[] = {
		// Positions
		0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f
	};
	GLfloat transparentVertices[] = {
		// Positions		//Normals					// Texture Coords (swapped y coordinates because texture is flipped upside down)
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f
	};

	//Quad, which will be used to display the texture, to which the scene is rendered
	GLfloat quadVertices[] = {
		// Positions   // TexCoords
		-1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	//Quad, which will be used to display the texture, to which the scene is rendered
	GLfloat cornerQuadVertices[] = {
		// Positions   // TexCoords
		-1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, 0.5f, 0.0f, 0.0f,
		-0.5f, 0.5f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,
		-0.5f, 0.5f, 1.0f, 0.0f,
		-0.5f, 1.0f, 1.0f, 1.0f
	};

	GLfloat cubeWithNormals[] = {
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
		0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
	};

	GLfloat planeVertices[] = {
		// Positions			 // Normals         // Texture Coords
		60.0f, -0.5f, 60.0f,	0.0f, 1.0f, 0.0f,	50.0f, 0.0f,
		-60.0f, -0.5f, 60.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
		-60.0f, -0.5f, -60.0f,	0.0f, 1.0f, 0.0f,	0.0f, 50.0f,

		60.0f, -0.5f, 60.0f,	0.0f, 1.0f, 0.0f,	50.0f, 0.0f,
		-60.0f, -0.5f, -60.0f,	0.0f, 1.0f, 0.0f,	0.0f, 50.0f,
		60.0f, -0.5f, -60.0f,	0.0f, 1.0f, 0.0f,	50.0f, 50.0f
	};

	GLfloat wallVertices[] = {
		// Positions		// Normals			// Texture Coords 
		0.0f, 0.5f, 0.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
		0.0f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f,
		1.0f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		0.0f, 0.5f, 0.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
		1.0f, -0.5f, 0.0f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		1.0f, 0.5f, 0.0f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f
	};
}