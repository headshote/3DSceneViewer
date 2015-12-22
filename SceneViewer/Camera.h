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
	explicit Camera();
	~Camera();

	GLfloat getPitch();
	GLfloat getYaw();
	void setPitch(const GLfloat value);
	void setYaw(const GLfloat value);

	GLfloat fov;

	void step(const GLdouble time, const GLdouble deltaTime);

	glm::mat4 getView(const GLboolean forceUpdate = false);
	glm::vec3 getPosition();
	glm::vec3 getCameraDirection();
protected:
private:
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 worldUp;

	GLfloat pitch;
	GLfloat yaw;

	glm::mat4 view;

	GLfloat cameraSpeed;

	void calculateDirection();
	
};

#endif	//CAMERA_H