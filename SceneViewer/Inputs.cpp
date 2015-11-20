#include "Inputs.h"

/*static*/ GLboolean Inputs::keys[1024];

/*static*/ GLfloat Inputs::lastX = 0;
/*static*/ GLfloat Inputs::lastY = 0;

/*static*/ GLfloat Inputs::xoffset = 0;
/*static*/ GLfloat Inputs::yoffset = 0;

/*static*/ GLboolean Inputs::firstMouse = true;

/*static*/ GLdouble Inputs::xScroll = 0;
/*static*/ GLdouble Inputs::yScroll = 0;

Inputs::Inputs()
{
}


Inputs::~Inputs()
{
}

/*static*/ void Inputs::onScroll(GLdouble xoffset, GLdouble yoffset)
{
	Inputs::xScroll = xoffset;
	Inputs::yScroll = yoffset;
}

/*static*/ void Inputs::step(GLdouble timeStamp, GLdouble timeDelta)
{
	Inputs::xScroll = 0.0f;
	Inputs::yScroll = 0.0f;
	Inputs::xoffset = 0.0f;
	Inputs::yoffset = 0.0f;
}

/*static*/ void Inputs::onMouseMove(GLfloat xpos, GLfloat ypos)
{
	if (Inputs::firstMouse)
	{
		Inputs::lastX = xpos;
		Inputs::lastY = ypos;
		Inputs::firstMouse = false;
	}

	//calculate the offset movement between the last and current frame:
	Inputs::xoffset = xpos - Inputs::lastX;
	Inputs::yoffset = Inputs::lastY - ypos; // Reversed since y-coordinates range from bottom to top
	Inputs::lastX = xpos;
	Inputs::lastY = ypos;

	GLfloat sensitivity = 0.05f;
	Inputs::xoffset *= sensitivity;
	Inputs::yoffset *= sensitivity;
}
