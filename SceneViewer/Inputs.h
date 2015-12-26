#pragma once

#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

class Inputs
{
public:
	static std::shared_ptr<Inputs> instance();

	//A necessery evil, have to have event callbacks as static methods, statics just redirect the event processing
	//to the Inputs singleton object
	static void glfwMousePositionCallback(GLFWwindow* window, GLdouble xpos, GLdouble ypos);
	static void glfwMouseScrollCallback(GLFWwindow* window, GLdouble xoffset, GLdouble yoffset);
	static void glfwMouseClicksCallback(GLFWwindow* window, GLint button, GLint action, GLint mode);
	static void glfwKeyCallback(GLFWwindow* window, GLint key, GLint scancode, GLint action, GLint mode);

	~Inputs();

	void step(GLdouble timeStamp, GLdouble timeDelta);

	GLboolean isKeyHeld(GLint key);
	GLboolean isKeyPressed(GLint key);
	GLboolean isKeyReleased(GLint key);

	GLboolean isMouseHeld();

	GLfloat mouseXOffset();
	GLfloat mouseYOffset();
	GLdouble mouseXScroll();
	GLdouble mouseYScroll();

private:
	static std::shared_ptr<Inputs> theInstance;

	//no copy constructors and assignments
	Inputs(const Inputs&);
	Inputs& operator=(const Inputs&);
	Inputs();

	std::vector<GLboolean> justPressedKeys;
	std::vector<GLboolean> justReleasedKeys;
	std::vector<GLboolean> heldKeys;

	GLfloat lastX;
	GLfloat lastY;
	GLfloat xoffset;
	GLfloat yoffset;
	GLboolean firstMouse;

	GLboolean mouseCapture;

	GLdouble xScroll;
	GLdouble yScroll;

	//Actual event processing goes on here
	void onScroll(GLdouble xoffset, GLdouble yoffse);
	void onMouseMove(GLfloat xpos, GLfloat ypos);
	void onMouseCLick(GLFWwindow* window, GLint button, GLint action, GLint mode);
	void onKey(GLint key, GLint scancode, GLint action, GLint mode);
};

