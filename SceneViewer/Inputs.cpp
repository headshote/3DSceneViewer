#include "Inputs.h"

/*static*/ std::shared_ptr<Inputs> Inputs::theInstance;

/*static*/ std::shared_ptr<Inputs> Inputs::instance()
{
	if (!theInstance.get())
		theInstance.reset(new Inputs());
	return theInstance;
}

/*static*/void Inputs::glfwMousePositionCallback(GLFWwindow* window, GLdouble xpos, GLdouble ypos)
{
	Inputs::instance()->onMouseMove((GLfloat)xpos, (GLfloat)ypos);
}


/*static*/void Inputs::glfwMouseScrollCallback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset)
{
	Inputs::instance()->onScroll(xoffset, yoffset);
}

/*static*/ void Inputs::glfwMouseClicksCallback(GLFWwindow* window, GLint button, GLint action, GLint mode)
{
	Inputs::instance()->onMouseCLick(window, button, action, mode);
}

/*static*/ void Inputs::glfwKeyCallback(GLFWwindow* window, GLint key, GLint scancode, GLint action, GLint mode)
{
	Inputs::instance()->onKey(key, scancode, action, mode);
}

Inputs::Inputs() :
	justPressedKeys(std::vector<GLboolean>(1024, false)),
	justReleasedKeys(std::vector<GLboolean>(1024, false)),
	heldKeys(std::vector<GLboolean>(1024, false)),
	lastX(0.0f),
	lastY(0.0f),
	xoffset(0.0f),
	yoffset(0.0f),
	firstMouse(true),
	mouseCapture(false),
	xScroll(0.0),
	yScroll(0.0)
{

}

Inputs::~Inputs()
{
}

void Inputs::step(GLdouble timeStamp, GLdouble timeDelta)
{
	xScroll = 0.0f;
	yScroll = 0.0f;
	xoffset = 0.0f;
	yoffset = 0.0f;

	for (GLuint i = 0; i < justReleasedKeys.size(); ++i)
		justReleasedKeys[i] = false;
	for (GLuint i = 0; i < justPressedKeys.size(); ++i)
		justPressedKeys[i] = false;
}

GLboolean Inputs::isKeyHeld(GLint key)
{
	return heldKeys[key];
}
GLboolean Inputs::isKeyPressed(GLint key)
{
	return justPressedKeys[key];
}
GLboolean Inputs::isKeyReleased(GLint key)
{
	return justReleasedKeys[key];
}

GLboolean Inputs::isMouseHeld()
{
	return mouseCapture;
}

GLfloat Inputs::mouseXOffset()
{
	return xoffset;
}
GLfloat Inputs::mouseYOffset()
{
	return yoffset;
}
GLdouble Inputs::mouseXScroll()
{
	return xScroll;
}
GLdouble Inputs::mouseYScroll()
{
	return yScroll;
}

void Inputs::onScroll(GLdouble xoffset, GLdouble yoffset)
{
	xScroll = xoffset;
	yScroll = yoffset;
}

void Inputs::onMouseMove(GLfloat xpos, GLfloat ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	//calculate the offset movement between the last and current frame:
	xoffset = xpos - lastX;
	yoffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	GLfloat sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;
}

void Inputs::onMouseCLick(GLFWwindow* window, int button, int action, int mode)
{
	//std::cout << "Mouse burron press: " << button << " action : " << action << " mode : " << mode << std::endl;
	if (button == GLFW_MOUSE_BUTTON_1 || button == GLFW_MOUSE_BUTTON_3)
	{
		mouseCapture = action == GLFW_PRESS;
		glfwSetInputMode(window, GLFW_CURSOR, mouseCapture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}
}

void Inputs::onKey(int key, int scancode, int action, int mode)
{
	//std::cout << "key press key: " << key << " scancode : " << scancode << " action : " << action << " mode : " << mode << std::endl;

	//A one-time event, upon releasing a key (so that it won't be done repetedly, while key is held)
	justReleasedKeys[key] = action == GLFW_RELEASE;
	//same for pressing, has it been pressed this frame, and not just held
	justPressedKeys[key] = action == GLFW_PRESS;


	if (action == GLFW_PRESS || action == GLFW_REPEAT)
		heldKeys[key] = true;
	else
		heldKeys[key] = false;
}
