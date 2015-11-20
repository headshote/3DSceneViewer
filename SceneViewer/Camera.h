#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Inputs.h"

class Camera
{
public:
	GLfloat getPitch();
	GLfloat getYaw();
	void setPitch(GLfloat value);
	void setYaw(GLfloat value);

	GLfloat fov;

	Camera();
	~Camera();

	void step(GLdouble time, GLdouble deltaTime);

	glm::mat4 getView(GLboolean forceUpdate = false);
	glm::vec3 getPosition();
	glm::vec3 getCameraDirection();
protected:
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 worldUp;

	GLfloat pitch;
	GLfloat yaw;

	glm::mat4 view;

	GLfloat cameraSpeed;

	void calculateDirection();
private:
	
};

#endif	//CAMERA_H