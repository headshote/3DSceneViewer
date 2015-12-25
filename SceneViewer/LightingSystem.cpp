#include "LightingSystem.h"

using namespace lighting;

LightingSystem::LightingSystem() : 
	dirLight(DirectionalLight{
		glm::vec3(-0.2f, -1.0f, -0.9f),
		glm::vec3(0.2f, 0.2f, 0.2f)
	}),
	pointLights(std::vector < PointLight > {
		PointLight{
			glm::vec3(0.7f, 0.2f, 2.0f),
			glm::vec3(0.35f, 0.05f, 0.15f)
		},
		PointLight{
			glm::vec3(1.2f, 0.2f, 0.5f),
			glm::vec3(1.75f, 1.75f, 1.75f)
		}, 
		PointLight{
			glm::vec3(35.5f, 0.0f, -37.33f),
			glm::vec3(2.45f, 2.45f, 2.45f)
		}, 
		PointLight{
			glm::vec3(41.0f, 10.5f, 35.0f),
			glm::vec3(0.35f, 0.35f, 0.35f)
		},
	}),
	spotLight(SpotLight{
		glm::vec3(),
		glm::vec3(),
		glm::vec3(0.9f, 0.9f, 0.9f)
	}), 
	kc(1.0f), 
	kl(0.09f), 
	kq(0.032f)
{

}

void LightingSystem::setSpotLightColor(const glm::vec3& color)
{
	spotLight.color = color;
}

void LightingSystem::setDirLightColor(const glm::vec3& color)
{
	dirLight.color = color;
}

DirectionalLight LightingSystem::getDirLight()
{
	return DirectionalLight(dirLight);
}

PointLight LightingSystem::getPointLight(GLuint id)
{
	return PointLight(pointLights[id]);
}

SpotLight LightingSystem::getSpotLight()
{
	return SpotLight(spotLight);
}

GLuint LightingSystem::getNumPointLights()
{
	return pointLights.size();
}

void LightingSystem::setCameraPosition(const glm::vec3& pos)
{
	spotLight.position = glm::vec3(pos);
}

void LightingSystem::setCameraDirection(const glm::vec3& dir)
{
	spotLight.direction = glm::vec3(dir);
}

void LightingSystem::setLightingParameters(const GLuint shader)
{
	glUseProgram(shader);

	setDirectionalLight(shader);
	GLuint i = 0;
	for (; i < pointLights.size(); i++)
		setPointLight(shader, i);
	glUniform1i(glGetUniformLocation(shader, "activeLights"), i);	//don't calculate the lights we haven't initialized
	setSpotLight(shader);
}

void LightingSystem::setPointightPosition(const GLuint id, const glm::vec3& position)
{
	pointLights[id].position = position;
}

/**
Lighting calculations for a directional (flobal light)
*/
void LightingSystem::setDirectionalLight(GLuint shaderProgram)
{
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.ambient"), dirLight.color.x * 0.2f, dirLight.color.y * 0.2f, dirLight.color.z * 0.2f);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.diffuse"), dirLight.color.x, dirLight.color.y, dirLight.color.z);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.specular"), dirLight.color.x * 2.0f, dirLight.color.y * 2.0f, dirLight.color.z * 2.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.direction"), dirLight.direction.x, dirLight.direction.y, dirLight.direction.z);
}

/**
Set all the params for a spotlight (transfered to a fragment shader via uniform)
*/
void LightingSystem::setPointLight(GLuint shaderProgram, GLuint lightId)
{
	glm::vec3 lightColor = pointLights[lightId].color;
	glm::vec3 lightPosition = pointLights[lightId].position;

	std::string iteration = std::to_string(lightId);
	std::string uniformName = "pointLights[" + iteration + "]";
	glUniform1f(glGetUniformLocation(shaderProgram, (uniformName + ".constant").c_str()), kc);
	glUniform1f(glGetUniformLocation(shaderProgram, (uniformName + ".linear").c_str()), kl);
	glUniform1f(glGetUniformLocation(shaderProgram, (uniformName + ".quadratic").c_str()), kq);
	glUniform3f(glGetUniformLocation(shaderProgram, (uniformName + ".ambient").c_str()), lightColor.x * 0.2f, lightColor.y * 0.2f, lightColor.z * 0.2f);
	glUniform3f(glGetUniformLocation(shaderProgram, (uniformName + ".diffuse").c_str()), lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(glGetUniformLocation(shaderProgram, (uniformName + ".specular").c_str()), lightColor.x * 2.0f, lightColor.y * 2.0f, lightColor.z * 2.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, (uniformName + ".position").c_str()), lightPosition.x, lightPosition.y, lightPosition.z);

	//calculate light volume
	GLfloat lightMax = std::fmaxf(std::fmaxf(lightColor.r, lightColor.g), lightColor.b);
	GLfloat radius = (-kl + std::sqrtf(kl * kl - 4 * kq * (kc - (256.0f / 5.0f) * lightMax))) / (2.0f * kq);
	glUniform1f(glGetUniformLocation(shaderProgram, (uniformName + ".radius").c_str()), radius);
}

/**
Set all the params for a spotlight (transfered to a fragment shader via uniform)
*/
void LightingSystem::setSpotLight(GLuint shaderProgram)
{
	glUniform1f(glGetUniformLocation(shaderProgram, "spotlight.constant"), kc);
	glUniform1f(glGetUniformLocation(shaderProgram, "spotlight.linear"), kl);
	glUniform1f(glGetUniformLocation(shaderProgram, "spotlight.quadratic"), kq);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotlight.ambient"), spotLight.color.x * 0.2f, spotLight.color.y * 0.2f, spotLight.color.z * 0.2f);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotlight.diffuse"), spotLight.color.x, spotLight.color.y, spotLight.color.z);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotlight.specular"), spotLight.color.x * 2.0f, spotLight.color.y * 2.0f, spotLight.color.z * 2.0f);

	glUniform3f(glGetUniformLocation(shaderProgram, "spotlight.position"), spotLight.position.x, spotLight.position.y, spotLight.position.z);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotlight.direction"), spotLight.direction.x, spotLight.direction.y, spotLight.direction.z);
	glUniform1f(glGetUniformLocation(shaderProgram, "spotlight.cutOff"), glm::cos(glm::radians(12.5f)));
	glUniform1f(glGetUniformLocation(shaderProgram, "spotlight.outerCutOff"), glm::cos(glm::radians(17.5f)));
}
