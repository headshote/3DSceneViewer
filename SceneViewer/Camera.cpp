#include "Camera.h"

glm::mat4 Camera::getView(GLboolean forceUpdate)
{
	if (forceUpdate)
		view = glm::lookAt(position, position + direction, worldUp);
	return view;
}

glm::vec3 Camera::getPosition()
{
	return position;
}

glm::vec3 Camera::getCameraDirection()
{
	return direction;
}

GLfloat Camera::getPitch()
{
	return pitch;
}

GLfloat Camera::getYaw()
{
	return yaw;
}

void Camera::setPitch(GLfloat value)
{
	pitch = value;
	calculateDirection();
}

void Camera::setYaw(GLfloat value)
{
	yaw = value;
	calculateDirection();
}

Camera::Camera()
{
	position = glm::vec3(0.0f, 0.0f, 3.0f);

	direction = glm::vec3(0.0f, 0.0f, -1.0f);

	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

	view = glm::lookAt(position, position + direction, worldUp);

	cameraSpeed = 3.0f;

	fov = 50.0f;

	pitch = 0.0f;
	yaw = -90.0f;	// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
}

Camera::~Camera()
{
}

void Camera::step(GLdouble time, GLdouble deltaTime)
{
	//Fov change on mouse scrolls
	if (fov >= 1.0f && fov <= 100.0f)
		fov -= (GLfloat)Inputs::yScroll;

	if (fov <= 1.0f)
		fov = 1.0f;

	if (fov >= 100.0f)
		fov = 100.0f;

	//Yaw, itch, rotatin camer fps-style on mouse movement
	yaw += Inputs::xoffset;
	pitch += Inputs::yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	calculateDirection();

	//Translating camera on key presses
	GLfloat camspeed = cameraSpeed;
	if (Inputs::keys[GLFW_KEY_LEFT_SHIFT])
		camspeed *= 10.5f;

	if (Inputs::keys[GLFW_KEY_W])
		position += camspeed * (GLfloat)deltaTime * direction;
	if (Inputs::keys[GLFW_KEY_A])
		position -= glm::normalize(glm::cross(direction, worldUp)) * camspeed * (GLfloat)deltaTime;
	if (Inputs::keys[GLFW_KEY_S])
		position -= camspeed * (GLfloat)deltaTime * direction;
	if (Inputs::keys[GLFW_KEY_D])
		position += glm::normalize(glm::cross(direction, worldUp)) * camspeed * (GLfloat)deltaTime;
	if (Inputs::keys[GLFW_KEY_SPACE])
		position += worldUp * camspeed * (GLfloat)deltaTime;
	if (Inputs::keys[GLFW_KEY_LEFT_CONTROL])
		position += -worldUp * camspeed * (GLfloat)deltaTime;

	view = glm::lookAt(position, position + direction, worldUp);
}

void Camera::calculateDirection()
{
	direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	direction.y = sin(glm::radians(pitch));
	direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	direction = glm::normalize(direction);
}