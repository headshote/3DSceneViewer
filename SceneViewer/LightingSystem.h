#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace lighting
{
	struct DirectionalLight
	{
		glm::vec3 direction;
		glm::vec3 color;
	};

	struct PointLight
	{
		glm::vec3 position;
		glm::vec3 color;
	};

	struct SpotLight
	{
		glm::vec3 position;
		glm::vec3 direction;
		glm::vec3 color;
	};

	class LightingSystem
	{
	public:
		LightingSystem();

		void setLightingParameters(const GLuint shader);

		void setCameraPosition(const glm::vec3& pos);
		void setCameraDirection(const glm::vec3& dir);

		void setDirLightColor(const glm::vec3& color);
		void setSpotLightColor(const glm::vec3& color);

		void setPointightPosition(const GLuint id, const glm::vec3& position);

		DirectionalLight getDirLight();
		PointLight getPointLight(GLuint id);
		SpotLight getSpotLight();

		GLuint getNumPointLights();

	private:
		DirectionalLight dirLight;
		std::vector<PointLight> pointLights;
		SpotLight spotLight;

		GLfloat kc;
		GLfloat kl;
		GLfloat kq;

		void setDirectionalLight(GLuint shaderProgram);
		void setPointLight(GLuint shaderProgram, GLuint lightId);
		void setSpotLight(GLuint shaderProgram);

	};
}
