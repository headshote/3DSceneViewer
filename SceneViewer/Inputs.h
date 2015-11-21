#pragma once

#include <GLFW/glfw3.h>
class Inputs
{
public:
	Inputs();
	~Inputs();

	static GLfloat lastX;
	static GLfloat lastY;
	static GLfloat xoffset;
	static GLfloat yoffset;
	static GLboolean firstMouse;

	static GLboolean mouseCapture;

	static GLdouble xScroll;
	static GLdouble yScroll;

	static GLboolean keys[1024];

	static void onScroll(GLdouble xoffset, GLdouble yoffse);
	static void step(GLdouble timeStamp, GLdouble timeDelta);
	static void onMouseMove(GLfloat xpos, GLfloat ypos);
};

