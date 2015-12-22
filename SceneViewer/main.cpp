#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <SOIL/SOIL.h>
#include <assimp/Importer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H  

#include "rendering.h"
#include "dataArrays.h"
#include "DataStructures.hpp"
#include "Shader.h"
#include "Camera.h"
#include "Inputs.h"
#include "Mesh.h"
#include "Model.h"
#include "RawPrimitive.h"

const GLint SCREEN_WIDTH = 1280;
const GLint SCREEN_HEIGHT = 720;

const GLuint SHADOW_WIDTH = 1536;
const GLuint SHADOW_HEIGHT = 1536;

const GLint NUM_FRAGMENT_SAMPLES = 4;

//---State vars
glm::vec3 spotLightColor(0.9f, 0.9f, 0.9f);

GLuint screenShaderId = 0;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//std::cout << "key press key: " << key << " scancode : " << scancode << " action : " << action << " mode : " << mode << std::endl;

	//A one-time event, upon releasing a key (so that it won't be done repetedly, while key is held)
	if (action == GLFW_RELEASE)
	{
		//Close window
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, GL_TRUE);

		//Switch between wireframe and fill mode
		if (key == GLFW_KEY_ENTER)
		{
			GLint polygonMode;
			glGetIntegerv(GL_POLYGON_MODE, &polygonMode);

			glPolygonMode(GL_FRONT_AND_BACK, polygonMode == GL_FILL ? GL_LINE : GL_FILL);			
		}

		//Flahslight on//off
		if (key == GLFW_KEY_F)
		{
			GLfloat clorValue = spotLightColor.x == 0.0f ? 0.5f : 0.0f;
			spotLightColor.x = clorValue;
			spotLightColor.y = clorValue;
			spotLightColor.z = clorValue;
		}

		//Switch between post-processing filters
		if (key >= GLFW_KEY_1 &&  key <= GLFW_KEY_8)
			screenShaderId = key - GLFW_KEY_1;

		if (key == GLFW_KEY_R)
			rendering::rearView = !rendering::rearView;

		//Model outlines on/off
		if (key == GLFW_KEY_Q)
		{
			rendering::highlightModels = !rendering::highlightModels;
			rendering::explodeMode = false;
			rendering::dotMode = false;
			rendering::renderNormals = false;
		}

		if (key == GLFW_KEY_G)
		{
			rendering::dotMode = !rendering::dotMode;
			rendering::explodeMode = false;
			rendering::highlightModels = false;
			rendering::renderNormals = false;
		}

		if (key == GLFW_KEY_X)
		{
			rendering::explodeMode = !rendering::explodeMode;
			rendering::dotMode = false;
			rendering::highlightModels = false;
			rendering::renderNormals = false;
		}

		if (key == GLFW_KEY_Z)
		{
			rendering::renderNormals = !rendering::renderNormals;
			rendering::dotMode = false;
			rendering::highlightModels = false;
			rendering::explodeMode = false;
		}

		if (key == GLFW_KEY_V)
			rendering::pointLightShadows = !rendering::pointLightShadows;

		if (key == GLFW_KEY_C)
		{
			rendering::deferredMode = !rendering::deferredMode;
			std::cout << (rendering::deferredMode ? "Entering deferred rendering mode" : "Entering straightforward rendering mode") << std::endl;
		}
	}
	
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		if (key == GLFW_KEY_LEFT_BRACKET)
		{
			rendering::hdrExposure -= 0.25f;
			std::cout << "hdr exposure: " << rendering::hdrExposure << std::endl;
		}
		if (key == GLFW_KEY_RIGHT_BRACKET)
		{
			rendering::hdrExposure += 0.25f;
			std::cout << "hdr exposure: " << rendering::hdrExposure << std::endl;
		}
	}

	Inputs::keys[key] = action != GLFW_RELEASE;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	Inputs::onMouseMove((GLfloat)xpos, (GLfloat)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Inputs::onScroll(xoffset, yoffset);	
}

GLFWwindow* setUpWindow(int width, int height)
{
	//instantiate the GLFW window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	//Next we're required to create a window object.
	GLFWwindow* window = glfwCreateWindow(width, height, "Scene Viewer", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	//Initialize GLEW before we call any OpenGL functions. 
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return nullptr;
	}

	glViewport(0, 0, width, height);

	//register the function with the proper callback via GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mode){
		std::cout << "Mouse burron press: " << button << " action : " << action << " mode : " << mode << std::endl;
		if (button == GLFW_MOUSE_BUTTON_1 || button == GLFW_MOUSE_BUTTON_3)
		{
			Inputs::mouseCapture = action == GLFW_PRESS;
			glfwSetInputMode(window, GLFW_CURSOR, Inputs::mouseCapture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
		}
	});

	return window;
}

void configureOpenGL()
{
	//Depth testing options
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);	//LEQUAL, instead of less, for that little oprimisation in cubemap shader to work

	//Stencil
	glDisable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	//Face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);	//discard back faces
	glFrontFace(GL_CCW); //Counter-clockwise winding order for faces that are facing us (hehe)

	//Influencing the point sizes in the vertex shader is disabled by default, enable it:
	glEnable(GL_PROGRAM_POINT_SIZE);  

	//Anti-aliasing
	//glfwWindowHint(GLFW_SAMPLES, NUM_FRAGMENT_SAMPLES);	//would have been useful, if we rendered directly to the window's default frame buffer
	glEnable(GL_MULTISAMPLE);
}

void blendingOn()
{
	//Blending, doesn't work with 'g-buffer'-type rendering (Deferred lighting/shading)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
}

void blendingOff()
{
	glDisable(GL_BLEND);
}

void printInfo()
{
	GLint nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;

	//Test translation with GLM
	glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 trans;
	trans = glm::translate(trans, glm::vec3(1.0f, 1.0f, 0.0f));
	vec = trans * vec;
	std::cout << vec.x << " " << vec.y << " " << vec.z << " " << std::endl;
}

void prepareModels(std::vector<Model>& models)
{
	Model& model = models[0];

	for (GLuint i = 0; i < 25; i++)
	{
		model.setTranslation(glm::vec3(8.0f * sin(0.25f * (GLfloat)i), 0.0f, 8.0f * cos(0.25f * (GLfloat)i)));
		model.setRotation(glm::vec3(0.0f, 1.0f, 0.0f), 45.0f + 2.0f * i);
		model.scheduleRendering();
	}

	for (GLuint i = 0; i < 45; i++)
	{
		model.setTranslation(glm::vec3(20.0f * sin(0.14f * (GLfloat)i), 1.0f, 20.0f * cos(0.14f * (GLfloat)i)));
		model.setRotation(glm::vec3(0.0f, 1.0f, 0.0f), 45.0f + 2.0f * i);
		model.scheduleRendering();
	}

	for (GLuint i = 0; i < 70; i++)
	{
		model.setTranslation(glm::vec3(30.0f * sin(0.09f * (GLfloat)i), 2.0f, 30.0f * cos(0.09f * (GLfloat)i)));
		model.setRotation(glm::vec3(0.0f, 1.0f, 0.0f), 45.0f + 2.0f * i);
		model.scheduleRendering();
	}

	model.flushScheduledInstances();

	Model& rock = models[3]; 
	for (GLuint i = 0; i < 25; i++)
	{
		rock.setTranslation(glm::vec3(15.75f * sin(0.25f * (GLfloat)i), 4.0f, 15.75f * cos(0.25f * (GLfloat)i)));
		rock.setRotation(glm::vec3(0.0f, 1.0f, 0.0f), 30.0f + 45.0f * i);
		rock.setRotation(glm::vec3(1.0f, 0.0f, 0.0f), 20.0f + 25.0f * i);
		rock.scheduleRendering();
	}
	rock.flushScheduledInstances();

	Model& planet = models[4]; 
	for (GLuint i = 0; i < 5; i++)
	{
		planet.setTranslation(glm::vec3(12.75f * sin(5.25f * (GLfloat)i), 4.0f, 12.75f * cos(5.25f * (GLfloat)i)));
		planet.setRotation(glm::vec3(0.0f, 1.0f, 0.0f), 45.0f + 2.0f * i);
		planet.setRotation(glm::vec3(1.0f, 0.0f, 0.0f), 20.0f + 25.0f * i);
		planet.scheduleRendering();
	}
	planet.flushScheduledInstances();
}

/**
	Makes the default FBO currently active, and clears it
*/
void clearScreen()
{
	//Clear default buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/**
	Sets up current frame buffer for rendering calls and clears it
*/
void clearFrameBuffer(GLuint FBO)
{
	//Clear current buffer
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void setUniformMaxtrix(GLuint shaderProgram, GLchar* uniformName, const glm::mat4& value)
{
	GLint uniform = glGetUniformLocation(shaderProgram, uniformName);
	glUniformMatrix4fv(uniform, 1, 
		GL_FALSE,	//transpose?
		glm::value_ptr(value));
}

/**
	Lighting calculations for a directional (flobal light)
*/
void directionalLight(GLuint shaderProgram, const glm::vec3& lightColor, const glm::vec3& lightDirection)
{
	//
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.ambient"), lightColor.x * 0.2f, lightColor.y * 0.2f, lightColor.z * 0.2f);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.diffuse"), lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.specular"), lightColor.x * 2.0f, lightColor.y * 2.0f, lightColor.z * 2.0f);
	glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.direction"), lightDirection.x, lightDirection.y, lightDirection.z);
}

/**
	Set all the params for a spotlight (transfered to a fragment shader via uniform)
*/
void pointLight(GLuint shaderProgram, const glm::vec3& lightColor, glm::vec3& lightPosition, GLuint lightId, GLfloat kc, GLfloat kl, GLfloat kq)
{
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
void spotLight(GLuint shaderProgram, const glm::vec3& lightColor, const glm::vec3& lightPosition, const glm::vec3& lightDirection, GLfloat kc, GLfloat kl, GLfloat kq)
{
	glUniform1f(glGetUniformLocation(shaderProgram, "spotlight.constant"), kc);
	glUniform1f(glGetUniformLocation(shaderProgram, "spotlight.linear"), kl);
	glUniform1f(glGetUniformLocation(shaderProgram, "spotlight.quadratic"), kq);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotlight.ambient"), lightColor.x * 0.2f, lightColor.y * 0.2f, lightColor.z * 0.2f);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotlight.diffuse"), lightColor.x, lightColor.y, lightColor.z);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotlight.specular"), lightColor.x * 2.0f, lightColor.y * 2.0f, lightColor.z * 2.0f);

	glUniform3f(glGetUniformLocation(shaderProgram, "spotlight.position"), lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform3f(glGetUniformLocation(shaderProgram, "spotlight.direction"), lightDirection.x, lightDirection.y, lightDirection.z);
	glUniform1f(glGetUniformLocation(shaderProgram, "spotlight.cutOff"), glm::cos(glm::radians(12.5f)));
	glUniform1f(glGetUniformLocation(shaderProgram, "spotlight.outerCutOff"), glm::cos(glm::radians(17.5f))); 
}

void setLightingParameters(GLuint shader, GLfloat currentTime, glm::vec3& directionalLightColor, glm::vec3& directionalLightDir, glm::vec3& spotLightColor, glm::vec3& cameraDirection,
	glm::vec3& cameraPostion, glm::vec3* pointLightPositions, glm::vec3* pointLightColors, GLuint numPointLights)
{
	glUseProgram(shader);

	//
	glUniform3f(glGetUniformLocation(shader, "viewerPos"), cameraPostion.x, cameraPostion.y, cameraPostion.z);

	if (rendering::explodeMode)
		glUniform1f(glGetUniformLocation(shader, "time"), currentTime);
	directionalLight(shader, directionalLightColor, directionalLightDir);
	GLuint i = 0;
	for (; i < numPointLights; i++)
		pointLight(shader, pointLightColors[i], pointLightPositions[i], i, 1.0f, 0.09f, 0.032f);
	glUniform1i(glGetUniformLocation(shader, "activeLights"), i);	//don't calculate the lights we haven't initialized
	spotLight(shader, spotLightColor, cameraPostion, cameraDirection, 1.0f, 0.09f, 0.032f);
}

/**
	Create a frame buffer, basically a texture, that we're going to render our scene to.
	Later we could map this texture to a single quad, the size of a whole screen and do some post-processing in shaders at sonic speeed.
	Used as an actual output buffer, because you need to give shaders a non-multisampled texture for texelation (as in applying it to actual fragments).
		colorBufferParams : true - for alpha-containing color texture. false - for 16bit precision float (usually, data storage)
*/
GLuint makeFrameBuffer(GLuint* colorBufferTextures, GLboolean* colorBufferParams, GLuint nColorBuffers, GLboolean colorOnly = false, GLboolean linearApprox = true)
{
	GLuint FBO;
	glGenFramebuffers(1, &FBO);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	//Store here the list of color attachments, that we'll use for this particular framebuffer
	std::vector<GLuint> attachments;

	//Texture attachment for the frame buffer
	glGenTextures(nColorBuffers, colorBufferTextures);
	for (GLuint i = 0; i < nColorBuffers; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBufferTextures[i]);
		if (colorBufferParams != NULL && colorBufferParams[i])	//alpha, texture storage
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		else	//noalpha, data buffer basically
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linearApprox ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linearApprox ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBufferTextures[i], 0);
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	if (!colorOnly)
	{
		//renderbuffer object being attached (Usually used as a stencil, depth buffer)
		GLuint RBO;
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	}

	//Give the list  of active color attachments for the current framebuffer
	glDrawBuffers(attachments.size(), &attachments[0]);

	//Correctness check
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		return 0;
	}
	
	//Bind back to the default framebuffer.
	//To make sure all rendering operations will have a visual impact on the main window 
	//we need to make the default framebuffer active again by binding to 0
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return FBO;
}

/**
Create a frame buffer, basically a multisampled texture texture, that we're going to render our scene to.
Later we could map this texture to a single quad, the size of a whole screen and do some post-processing in shaders at sonic speeed
*/
GLuint makeMultisampleFrameBuffer(GLuint* colorBufferTexture, GLuint nColorBuffers, GLuint samples = NUM_FRAGMENT_SAMPLES)
{
	GLuint FBO;
	glGenFramebuffers(1, &FBO);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	//Store here the list of color attachments, that we'll use for this particular framebuffer
	std::vector<GLuint> attachments;

	//Texture attachment for the frame buffer
	glGenTextures(nColorBuffers, colorBufferTexture);
	for (GLuint i = 0; i < nColorBuffers; i++)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorBufferTexture[i]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, GL_TRUE);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, colorBufferTexture[i], 0);
		attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
	}

	//renderbuffer object being attached (Usually used as a stencil, depth buffer)
	GLuint RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	glDrawBuffers(nColorBuffers, &attachments[0]);

	//Correctness check
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::MULTISAMPLE_FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		return 0;
	}

	//Bind back to the default framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return FBO;
}

/**
	Blits the colorbuffer from the multisampled FBO to the main FBO's color buffer (normal texture, that can be used for rendering)
*/
void blitMSampledScene(GLuint multisampleFBO, GLuint sceneFBO)
{
	//Blit the multisampled frame buffer (where we have rendered the scene), to a non-multisampled frame buffer,
	//which texture will be mapped to a full-screen quad
	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sceneFBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

/**	
	Using gauss blur, make multiple render calls, to render ping-pong buffers into a main fullscreen quad,
	exchanging their color textures, and blurring the brigtness buffer texture of the main rendering buffer
	Perfrom this before rendering the scene texture to the fullscreen quad
*/
void blurSceneBrightnessTextr(GLuint blurShader, GLuint brightnessTexture, GLuint quadVao, GLuint nQuadVertices, 
	GLuint* pingpongFBOs, GLuint* pingpongTextrs)
{
	//Blur the brigtness texture (with gauss blur)
	GLboolean horizontal = true, first_iteration = true;
	GLuint amount = 10;
	glUseProgram(blurShader);
	for (GLuint i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBOs[horizontal]);

		glUniform1i(glGetUniformLocation(blurShader, "horizontal"), horizontal);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? brightnessTexture : pingpongTextrs[!horizontal]);

		glBindVertexArray(quadVao);
		glDrawArrays(GL_TRIANGLES, 0, nQuadVertices);

		horizontal = !horizontal;
		first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
Renders the contects of a frame buffer (given by it's buffer texture)
into a quad (whose vertices are given as a parameter,
and which is supposed to be in normalized device coordinates, taking full screen),
using simple screen shader (no transforms
*/
void renderFrameBufferToQuad(GLuint shader, GLuint bufferTexture, GLuint brightnessTexture, GLuint quadVao, GLuint nQuadVertices)
{
	//activate the default buffer (screen buffer)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);

	//So that polygon mode for the quad is always fill
	GLint oldPolygonMode;
	glGetIntegerv(GL_POLYGON_MODE, &oldPolygonMode);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Render the quad with the frame buffer texture on it
	glUseProgram(shader);
	glUniform1f(glGetUniformLocation(shader, "exposure"), rendering::hdrExposure);

	glUniform1i(glGetUniformLocation(shader, "screenTexture"), 0);
	glUniform1i(glGetUniformLocation(shader, "brightnessTexture"), 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, brightnessTexture);

	glBindVertexArray(quadVao);
	glDrawArrays(GL_TRIANGLES, 0, nQuadVertices);

	//unbind objects
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//retrun to whatever polygon mode we used on the actual rendering
	glPolygonMode(GL_FRONT_AND_BACK, oldPolygonMode);
	glEnable(GL_DEPTH_TEST);
}

void setShaderTransforms(GLuint shaderProgram, GLuint outlineShader, GLuint theNormalsShader, 
	GLuint batchSahder, GLuint batchNormalsShader, GLuint batchExplodeShader,
	GLuint lightSourceShader, GLuint skyboxShader, const glm::mat4& projection, const glm::mat4& view, 
	const glm::mat4& lightSpaceMatrix, GLuint shadowmapTexture, GLuint pointShadowmapTexture, GLfloat pLightFarPlane,
	GLuint gBufferShader, GLuint gBufferBatchShader, GLuint deferredLightingShader, const glm::vec3 cameraPostion)
{
	glUseProgram(shaderProgram);
	setUniformMaxtrix(shaderProgram, "projection", projection);
	setUniformMaxtrix(shaderProgram, "view", view);
	setUniformMaxtrix(shaderProgram, "lightSpaceMatrix", lightSpaceMatrix);
	glActiveTexture(GL_TEXTURE16);
	glBindTexture(GL_TEXTURE_2D, shadowmapTexture);
	glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 16);
	glActiveTexture(GL_TEXTURE17);
	glBindTexture(GL_TEXTURE_CUBE_MAP, pointShadowmapTexture);
	glUniform1i(glGetUniformLocation(shaderProgram, "pointShadowMap"), 17);
	glUniform1f(glGetUniformLocation(shaderProgram, "pointLightFarPlane"), pLightFarPlane);
	glUniform1i(glGetUniformLocation(shaderProgram, "pointLightShadows"), rendering::pointLightShadows);

	glUseProgram(outlineShader);
	setUniformMaxtrix(outlineShader, "projection", projection);
	setUniformMaxtrix(outlineShader, "view", view);

	glUseProgram(theNormalsShader);
	setUniformMaxtrix(theNormalsShader, "projection", projection);
	setUniformMaxtrix(theNormalsShader, "view", view);

	glUseProgram(lightSourceShader);
	setUniformMaxtrix(lightSourceShader, "projection", projection);
	setUniformMaxtrix(lightSourceShader, "view", view);

	glUseProgram(batchSahder);
	setUniformMaxtrix(batchSahder, "projection", projection);
	setUniformMaxtrix(batchSahder, "view", view);
	setUniformMaxtrix(batchSahder, "lightSpaceMatrix", lightSpaceMatrix);
	glActiveTexture(GL_TEXTURE16);
	glBindTexture(GL_TEXTURE_2D, shadowmapTexture);
	glUniform1i(glGetUniformLocation(batchSahder, "shadowMap"), 16);
	glActiveTexture(GL_TEXTURE17);
	glBindTexture(GL_TEXTURE_CUBE_MAP, pointShadowmapTexture);
	glUniform1i(glGetUniformLocation(batchSahder, "pointShadowMap"), 17);
	glUniform1f(glGetUniformLocation(batchSahder, "pointLightFarPlane"), pLightFarPlane);
	glUniform1i(glGetUniformLocation(batchSahder, "pointLightShadows"), rendering::pointLightShadows);

	glUseProgram(batchNormalsShader);
	setUniformMaxtrix(batchNormalsShader, "projection", projection);
	setUniformMaxtrix(batchNormalsShader, "view", view);

	glUseProgram(batchExplodeShader);
	setUniformMaxtrix(batchExplodeShader, "projection", projection);
	setUniformMaxtrix(batchExplodeShader, "view", view);

	glUseProgram(gBufferShader);
	setUniformMaxtrix(gBufferShader, "projection", projection);
	setUniformMaxtrix(gBufferShader, "view", view);
	setUniformMaxtrix(gBufferShader, "lightSpaceMatrix", lightSpaceMatrix);
	glUniform3f(glGetUniformLocation(gBufferShader, "viewerPos"), cameraPostion.x, cameraPostion.y, cameraPostion.z);


	glUseProgram(gBufferBatchShader);
	setUniformMaxtrix(gBufferBatchShader, "projection", projection);
	setUniformMaxtrix(gBufferBatchShader, "view", view);
	setUniformMaxtrix(gBufferBatchShader, "lightSpaceMatrix", lightSpaceMatrix);
	glUniform3f(glGetUniformLocation(gBufferBatchShader, "viewerPos"), cameraPostion.x, cameraPostion.y, cameraPostion.z);


	glUseProgram(deferredLightingShader);
	glActiveTexture(GL_TEXTURE16);
	glBindTexture(GL_TEXTURE_2D, shadowmapTexture);
	glUniform1i(glGetUniformLocation(deferredLightingShader, "shadowMap"), 16);
	glActiveTexture(GL_TEXTURE17);
	glBindTexture(GL_TEXTURE_CUBE_MAP, pointShadowmapTexture);
	glUniform1i(glGetUniformLocation(deferredLightingShader, "pointShadowMap"), 17);
	glUniform1f(glGetUniformLocation(deferredLightingShader, "pointLightFarPlane"), pLightFarPlane);
	glUniform1i(glGetUniformLocation(deferredLightingShader, "pointLightShadows"), rendering::pointLightShadows);

	//Cubemap shouldn't be translated with player's movement, shoud be in place,
	//so we remove translation component from the view matrix
	glUseProgram(skyboxShader);
	setUniformMaxtrix(skyboxShader, "projection", projection);
	setUniformMaxtrix(skyboxShader, "view", glm::mat4(glm::mat3(view)));
}

/**
	Rendering models, reuse loaded models from hdd, just transform them and render whatever amount of times during the main loop iteration
*/
void renderCalls(GLuint shaderProgram, GLuint batchShaderProgram, GLuint outlineShader, GLuint outlineBatchShader, GLuint lightSourceShader, std::vector<Model>& models,
	std::vector<Model>& primitives, const glm::vec3* pointLightPositions, const glm::vec3& directionalLightDir)
{
	glEnable(GL_CULL_FACE);	//3d meshes, backface cull
	glUseProgram(batchShaderProgram);

	Model& model = models[0];

	if (rendering::highlightModels)
		models[3].batchRenderScheduledInstances(batchShaderProgram);// model.batchRenderOutlined(batchShaderProgram, outlineBatchShader);
	else
		models[3].batchRenderScheduledInstances(batchShaderProgram);

	if (rendering::highlightModels)
		models[4].batchRenderScheduledInstances(batchShaderProgram);// model.batchRenderOutlined(batchShaderProgram, outlineBatchShader);
	else
		models[4].batchRenderScheduledInstances(batchShaderProgram);

	if (rendering::highlightModels)
		model.batchRenderScheduledInstances(batchShaderProgram);// model.batchRenderOutlined(batchShaderProgram, outlineBatchShader);
	else
		model.batchRenderScheduledInstances(batchShaderProgram);

	glUseProgram(shaderProgram);

	model.setTranslation(glm::vec3(5.25f * sin(0.5f * (GLfloat)glfwGetTime()), 0.0f, 5.25f * cos(0.5f * (GLfloat)glfwGetTime())));
	model.setRotation(glm::vec3(0.0f, 1.0f, 0.0f), 45.0f + 90.0f * (GLfloat)glfwGetTime());
	if (rendering::highlightModels)
		model.drawOutlined(shaderProgram, outlineShader);
	else
		model.drawCall(shaderProgram);

	glDisable(GL_CULL_FACE);	//2d quads, no cull

	models[6].setTranslation(glm::vec3(0.0f, 0.5f, 0.0f));
	models[6].setRotation(glm::vec3(1.0f, 0.0f, 0.0f), 0.0f);
	models[6].drawCall(shaderProgram);
	models[6].setTranslation(glm::vec3(0.0f, 0.0f, 0.52f));
	models[6].setRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
	models[6].drawCall(shaderProgram);

	models[7].setTranslation(glm::vec3(1.1f, 0.5f, 0.0f));
	models[7].setRotation(glm::vec3(1.0f, 0.0f, 0.0f), 0.0f); 
	models[7].drawCall(shaderProgram);
	models[7].setTranslation(glm::vec3(1.1f, 0.0f, 0.52f));
	models[7].setRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
	models[7].drawCall(shaderProgram);

	models[8].setTranslation(glm::vec3(-1.1f, 0.5f, 0.0f));
	models[8].setRotation(glm::vec3(1.0f, 0.0f, 0.0f), 0.0f);
	models[8].drawCall(shaderProgram);
	models[8].setTranslation(glm::vec3(-1.1f, 0.0f, 0.52f));
	models[8].setRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
	models[8].drawCall(shaderProgram);

	models[9].setTranslation(glm::vec3(-2.1f, 0.5f, 0.0f));
	models[9].setRotation(glm::vec3(1.0f, 0.0f, 0.0f), 0.0f);
	models[9].drawCall(shaderProgram);
	models[9].setTranslation(glm::vec3(-2.1f, 0.0f, 0.52f));
	models[9].setRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
	models[9].drawCall(shaderProgram);

	models[5].drawCall(shaderProgram);

	models[1].drawCall(shaderProgram);
	
	models[2].drawCall(shaderProgram);

	//Render lightsources (as wireframe)
	glUseProgram(lightSourceShader);
	GLuint i = 0;
	for ( auto iterator = primitives.begin(); iterator != primitives.end(); i++, iterator++)
	{
		iterator->setTranslation(pointLightPositions[i]);
		iterator->drawCall(lightSourceShader);
	}
	primitives[0].setTranslation(directionalLightDir * (-26.0f));
	primitives[0].drawCall(lightSourceShader);
}

GLuint createCubeMap(const std::vector<GLchar*>& textures_faces, GLboolean gammacorrection = false)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	//Set texture for each face of a cubemap
	int width, height;
	unsigned char* image;
	for (GLuint i = 0; i < textures_faces.size(); i++)
	{
		image = SOIL_load_image(textures_faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, 
			gammacorrection ? GL_SRGB : GL_RGB,
			width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
		);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}

void activateSkyboxTexture(GLuint cubemapTexture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
}

void renderSkybox(GLuint skyboxShader, GLuint skyboxVAO, GLuint cubemapTexture)
{
	glUseProgram(skyboxShader);
	glDepthMask(GL_FALSE);

	glBindVertexArray(skyboxVAO);
	activateSkyboxTexture(cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, sizeof(dataArrays::skyboxVertices));
	glBindVertexArray(0);
	glDepthMask(GL_TRUE);
}

ShadowMap generateDirShadowRBuffer()
{
	//frame buffer
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO); 
	
	//depth buffer texture
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//attach the texture to the frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//We only need the depth information when rendering the scene from the light's perspective 
	//so there is no need for a color buffer. A framebuffer object however is not complete without a color buffer 
	//so we need to explicitly tell OpenGL we're not going to render any color data. 
	//We do this by setting both the read and draw buffer to GL_NONE
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ShadowMap shadowmap;
	shadowmap.shadowmapFBO = depthMapFBO;
	shadowmap.shadowmapTexture = depthMap;
	return shadowmap;
}

glm::mat4 setTransformationsForDirShadows(GLuint shadowMapShader, glm::vec3& lightPosition)
{
	glm::mat4 projection = glm::ortho(-75.0f, 75.0f, -75.0f, 75.0f, 0.1f, 83.5f);

	glm::mat4 view = glm::lookAt(lightPosition, glm::vec3(0.0f), glm::vec3(1.0f));

	glm::mat4 lightSpaceMatrix = projection * view;

	glUseProgram(shadowMapShader);
	setUniformMaxtrix(shadowMapShader, "lightSpaceMatrix", lightSpaceMatrix);

	return lightSpaceMatrix;
}

ShadowMap genPointShadowMap()
{
	//frame buffer
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	GLuint depthCubemap;
	glGenTextures(1, &depthCubemap);

	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (GLuint i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//Normally we'd attach a single face of a cubemap texture to the framebuffer object and render the scene 6 times, 
	//each time switching the depth buffer target of the framebuffer to a different cubemap face. 
	//Since we're going to use a geometry shader that allows us to render to all faces in a single pass
	//we can directly attach the cubemap as a framebuffer's depth attachment using glFramebufferTexture: 
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ShadowMap shadowmap;
	shadowmap.shadowmapFBO = depthMapFBO;
	shadowmap.shadowmapTexture = depthCubemap;
	return shadowmap;
}

GLfloat setPointShadowTransforms(GLuint pointShadowMapShader, glm::vec3& lightPos)
{
	GLfloat aspect = (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT;
	GLfloat near = 0.05f;
	GLfloat far = 30.0f;
	glm::mat4 shadowProj = glm::perspective(90.0f, aspect, near, far);

	std::vector<glm::mat4> lightSpaceMatrices = {
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0))
	};

	glUseProgram(pointShadowMapShader);
	for (GLuint i = 0; i < lightSpaceMatrices.size(); i++)
		setUniformMaxtrix(pointShadowMapShader, (GLchar*)("lightSpaceTransforms[" + std::to_string(i) + "]").c_str(), lightSpaceMatrices[i]);
	glUniform3f(glGetUniformLocation(pointShadowMapShader, "lightPosition"), lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(glGetUniformLocation(pointShadowMapShader, "far_plane"), far);

	return far;
}

void renderGBufferData(GLuint deferredLghtShdr, GLuint* gBufferTextures, GLuint nTextures, GLuint quadVao, GLuint nQuadVertices)
{
	//activate the default buffer (screen buffer)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);

	GLint oldPolygonMode;
	glGetIntegerv(GL_POLYGON_MODE, &oldPolygonMode);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//Render the quad with the frame buffer texture on it
	glUseProgram(deferredLghtShdr);

	glUniform1i(glGetUniformLocation(deferredLghtShdr, "gPosition"), 0);
	glUniform1i(glGetUniformLocation(deferredLghtShdr, "gNormal"), 1);
	glUniform1i(glGetUniformLocation(deferredLghtShdr, "gTangent"), 2);
	glUniform1i(glGetUniformLocation(deferredLghtShdr, "gBitangent"), 3);
	glUniform1i(glGetUniformLocation(deferredLghtShdr, "gAlbedoSpec"), 4);
	glUniform1i(glGetUniformLocation(deferredLghtShdr, "glightSpacePosition"), 5);

	for (GLuint i = 0; i < nTextures; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, gBufferTextures[i]);
	}

	glBindVertexArray(quadVao);
	glDrawArrays(GL_TRIANGLES, 0, nQuadVertices);

	//unbind objects
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//retrun to whatever polygon mode we used on the actual rendering
	glPolygonMode(GL_FRONT_AND_BACK, oldPolygonMode);
	glEnable(GL_DEPTH_TEST);
}

void loadTTFont(std::map<GLchar, Character>& charMap, GLchar* filePath = "fonts/arial.ttf", GLuint fontSize = 48)
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	FT_Face face;
	if (FT_New_Face(ft, filePath, 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	//The function sets the font's width and height parameters. 
	//Setting the width to 0 lets the face dynamically calculate the width based on the given height. 
	FT_Set_Pixel_Sizes(face, 0, fontSize);

	//Pre-laod 128 chars to opengl textures
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
			);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		charMap.insert(std::pair<GLchar, Character>(c, character));
	}
	
	//done, release resources
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

GLuint* generateFontQuad()
{
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	GLuint* retV = new GLuint[2];
	retV[0] = VAO;
	retV[1] = VBO;
	return retV;
}

void renderText(GLuint shader, GLuint quadVAO, GLuint quadVBO, std::map<GLchar, Character>& fontCharMap, const std::string& text, GLfloat x, GLfloat y, const glm::vec3& color)
{
	glm::mat4 projection = glm::ortho(0.0f, (GLfloat) SCREEN_WIDTH, 0.0f, (GLfloat) SCREEN_HEIGHT);

	glUseProgram(shader);

	glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(quadVAO);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = fontCharMap[*c];

		GLfloat xpos = x + ch.bearing.x;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y);

		GLint w = ch.size.x;
		GLint h = ch.size.y;
		// Update VBO for each character
		GLfloat vertices[24] = {
			 xpos, ypos + h, 0.0, 0.0 ,
			 xpos, ypos, 0.0, 1.0 ,
			 xpos + w, ypos, 1.0, 1.0 ,

			 xpos, ypos + h, 0.0, 0.0 ,
			 xpos + w, ypos, 1.0, 1.0 ,
			 xpos + w, ypos + h, 1.0, 0.0 
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureId);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (ch.advance >> 6); // divide by 64 using bitshift
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

int main()
{
	GLFWwindow* window = setUpWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
	if (window == nullptr)
	{
		return -1;
	}

	configureOpenGL();

	printInfo();

	//Compile separate shaders for normal objects, and a debug drawing of lightsources (wireframed)
	//Scene shaders
	std::shared_ptr<Shader> theBlinnShader(new Shader("shaders/vertex.glsl", "shaders/fragmentBlinnPong.glsl"));
	std::shared_ptr<Shader> theBlinnExplodeShader(new Shader("shaders/vertexExplode.glsl", "shaders/fragmentBlinnPong.glsl", "shaders/geometryExplode.glsl"));

	std::shared_ptr<Shader> lightSourceShader(new Shader("shaders/rawcolorvertex.glsl", "shaders/rawcolorfragment.glsl"));
	std::shared_ptr<Shader> outlineShader(new Shader("shaders/outlineVertex.glsl", "shaders/outlineFragment.glsl"));
	//post-processing shaders
	std::shared_ptr<Shader> screenShader(new Shader("shaders/screenVertex.glsl", "shaders/screenFragment.glsl"));
	std::shared_ptr<Shader> screenShaderBlur(new Shader("shaders/screenVertex.glsl", "shaders/screenFragmentBlur.glsl"));
	std::shared_ptr<Shader> screenShaderEdge(new Shader("shaders/screenVertex.glsl", "shaders/screenFragmentEdge.glsl"));
	std::shared_ptr<Shader> screenShaderGreyScale(new Shader("shaders/screenVertex.glsl", "shaders/screenFragmentGrayScale.glsl"));
	std::shared_ptr<Shader> screenShaderInversion(new Shader("shaders/screenVertex.glsl", "shaders/screenFragmentInversion.glsl"));
	std::shared_ptr<Shader> screenShaderSharpen(new Shader("shaders/screenVertex.glsl", "shaders/screenFragmentKernel.glsl"));
	std::shared_ptr<Shader> screenShaderHalves(new Shader("shaders/screenVertex.glsl", "shaders/screenFragmentHalves.glsl"));
	std::shared_ptr<Shader> screenShaderNoBloom(new Shader("shaders/screenVertex.glsl", "shaders/screenFragmentNoBloomy.glsl"));
	std::vector< std::shared_ptr<Shader> > screenShaders = { screenShader, screenShaderBlur, screenShaderEdge, 
		screenShaderGreyScale, screenShaderInversion, screenShaderSharpen, screenShaderHalves, screenShaderNoBloom };
	//sky box
	std::shared_ptr<Shader> skyBoxShader(new Shader("shaders/skyBoxVertex.glsl", "shaders/skyBoxFragment.glsl"));
	//debug rendering of the mesh normals
	std::shared_ptr<Shader> theNormalsShader(new Shader("shaders/normalsVertex.glsl", "shaders/normalsFragment.glsl", "shaders/normalsGeometry.glsl"));
	//batched rendering
	std::shared_ptr<Shader> theBlinnBatchShader(new Shader("shaders/batchVertex.glsl", "shaders/fragmentBlinnPong.glsl"));
	std::shared_ptr<Shader> theBlinnBatchExplodeShader(new Shader("shaders/batchVertexExplode.glsl", "shaders/fragmentBlinnPong.glsl", "shaders/geometryExplode.glsl"));
	std::shared_ptr<Shader> outlineBatchShader(new Shader("shaders/batchOutlineVertex.glsl", "shaders/outlineFragment.glsl"));
	std::shared_ptr<Shader> theBatchNormalsShader(new Shader("shaders/batchNormalsVertex.glsl", "shaders/normalsFragment.glsl", "shaders/normalsGeometry.glsl"));
	//for directional shadow mapping
	std::shared_ptr<Shader> shadowMapShader(new Shader("shaders/shadowVertex.glsl", "shaders/shadowFragment.glsl"));
	std::shared_ptr<Shader> shadowMapBatchShader(new Shader("shaders/shadowBatchVertex.glsl", "shaders/shadowFragment.glsl"));
	//for point shadow mapping
	std::shared_ptr<Shader> pointShadowMapShader(new Shader("shaders/pointShadowVertex.glsl", "shaders/pointShadowFragment.glsl", "shaders/pointShadowGeometry.glsl"));
	std::shared_ptr<Shader> pointShadowMapBatchShader(new Shader("shaders/pointShadowBatchVertex.glsl", "shaders/pointShadowFragment.glsl", "shaders/pointShadowGeometry.glsl"));
	//gauss blur
	std::shared_ptr<Shader> gaussBlurShdr(new Shader("shaders/gaussBlurVertex.glsl", "shaders/gaussBlurFragment.glsl"));
	//deferred rendering g-buffer shaders
	std::shared_ptr<Shader> thegBuffShader(new Shader("shaders/vertex.glsl", "shaders/gBufferFragment.glsl"));
	std::shared_ptr<Shader> thegBuffBatchShader(new Shader("shaders/batchVertex.glsl", "shaders/gBufferFragment.glsl"));
	std::shared_ptr<Shader> theDeferredtingShader(new Shader("shaders/screenVertex.glsl", "shaders/deferredLightingFragment.glsl"));
	//text rendering shader
	std::shared_ptr<Shader> textShader(new Shader("shaders/glyphVertex.glsl", "shaders/glyphFrahment.glsl"));

	std::shared_ptr<Camera> theCamera(new Camera());

	//Prepare frame buffer to render to
	GLuint colorBufferTexture[2];
	GLuint msColorBufferTexture[2];
	//since we're using gamma correction, use the srgb format for the texture, to which the scene is rendered, and which is rendered in the final buffer,
	//for which we enable gamma correction
	GLuint sceneFBO = makeFrameBuffer(colorBufferTexture, NULL, sizeof(colorBufferTexture) / sizeof(GLuint));
	GLuint multisampleFBO = makeMultisampleFrameBuffer(msColorBufferTexture, sizeof(msColorBufferTexture) / sizeof(GLuint));

	//for Gauss blur, for the Bloom effect on the bright lights sources/lit areas
	GLuint pingpongFBOs[2];
	GLuint pingponTextures[2];
	pingpongFBOs[0] = makeFrameBuffer(pingponTextures, NULL, 1, true);
	pingpongFBOs[1] = makeFrameBuffer(pingponTextures + 1, NULL, 1, true);

	//G-buffer, for deferred rendering
	GLuint gBufferTextures[6];	//1. position, 2. normal, 3. tangent, 4. bitangent, 5. diffuse+spec, 6. lightSpacePosition
	GLboolean gBufferTextrsParams[] = { false, false, false, false, true, true };
	GLuint gBuffer = makeFrameBuffer(gBufferTextures, gBufferTextrsParams, sizeof(gBufferTextures) / sizeof(GLuint), false, false);

	//This vao, contains the quad in NDC, which will have the FBO texture applied to it
	RawPrimitive rq(dataArrays::quadVertices, sizeof(dataArrays::quadVertices));
	GLuint renderingQuad = rq.getVAO();
	RawPrimitive rmq(dataArrays::cornerQuadVertices, sizeof(dataArrays::cornerQuadVertices));
	GLuint renderingMiniQuad = rmq.getVAO();

	//Load the models
	std::vector<Model> models;
	Model model1("models/nanosuit/nanosuit.obj");
	model1.setScale(glm::vec3(0.33f, 0.33f, 0.33f));
	models.push_back(model1);
	GLuint grassTexture[] = { loadTexture("textures/grass.png", true, false), loadTexture("textures/mt_specular.png") };
	Model model2(new Mesh(dataArrays::transparentVertices, sizeof(dataArrays::transparentVertices), grassTexture, sizeof(grassTexture) / sizeof(GLuint)));
	model2.setTranslation(glm::vec3(-1.5f, 0.0f, -0.48f));
	models.push_back(model2);
	GLuint glassTexture[] = { loadTexture("textures/blending_transparent_window.png", false, false), loadTexture("textures/mt_specular.png") };
	Model model3(new Mesh(dataArrays::transparentVertices, sizeof(dataArrays::transparentVertices), glassTexture, sizeof(glassTexture) / sizeof(GLuint), 0));
	model3.setTranslation(glm::vec3(0.5f, 0.0f, -0.48f));
	models.push_back(model3);
	models.push_back(Model("models/rock/rock.obj"));
	models.push_back(Model("models/planet/planet.obj"));
	//5
	GLuint metalTexture[] = { loadTexture("textures/metal1.jpg", false, false), loadTexture("textures/metal1_specular.png") };
	models.push_back(Model(new Mesh(dataArrays::planeVertices, sizeof(dataArrays::planeVertices), metalTexture, sizeof(metalTexture) / sizeof(GLuint))));
	//6
	GLuint brickTexture[] = { loadTexture("textures/brickwall.jpg", false, false), loadTexture("textures/mt_specular.png"), loadTexture("textures/brickwall_normal.jpg") };
	models.push_back(Model(new Mesh(dataArrays::wallVertices, sizeof(dataArrays::wallVertices), brickTexture, sizeof(brickTexture) / sizeof(GLuint))));
	//7
	GLuint brickPlainTexture[] = { loadTexture("textures/brickwall.jpg", false, false), loadTexture("textures/mt_specular.png") };
	models.push_back(Model(new Mesh(dataArrays::wallVertices, sizeof(dataArrays::wallVertices), brickPlainTexture, sizeof(brickPlainTexture) / sizeof(GLuint))));
	//8
	GLuint brickParallaxTexture[] = { 
		loadTexture("textures/parallax_brix/bricks2.jpg", false, false),
		loadTexture("textures/parallax_brix/specular.png"),
		loadTexture("textures/parallax_brix/bricks2_normal.jpg"),
		loadTexture("textures/parallax_brix/bricks2_disp.jpg") };
	models.push_back(Model(new Mesh(dataArrays::wallVertices, sizeof(dataArrays::wallVertices), brickParallaxTexture, sizeof(brickParallaxTexture) / sizeof(GLuint))));
	//9
	GLuint tbParallaxTexture[] = {
		loadTexture("textures/woodbox/wood.png", false, false),
		loadTexture("textures/woodbox/specular.png"),
		loadTexture("textures/woodbox/toy_box_normal.png"),
		loadTexture("textures/woodbox/toy_box_disp.png") };
	models.push_back(Model(new Mesh(dataArrays::wallVertices, sizeof(dataArrays::wallVertices), tbParallaxTexture, sizeof(tbParallaxTexture) / sizeof(GLuint))));

	//Pre-cache transforms for a lot of model instances
	prepareModels(models);

	//Cube map
	GLuint cubeTexture = createCubeMap(std::vector<GLchar*> {"textures/cubemap/mnight_rt.jpg", 
		"textures/cubemap/mnight_lf.jpg", "textures/cubemap/mnight_up.jpg", "textures/cubemap/mnight_dn.jpg", 
		"textures/cubemap/mnight_bk.jpg", "textures/cubemap/mnight_ft.jpg" });
	//when we pass vec3 to RawPrimitive constructor, it loads vertices as if 3d positions for a primitive, that is supposed to be rendered in one color
	RawPrimitive sb(dataArrays::skyboxVertices, sizeof(dataArrays::skyboxVertices), glm::vec3());
	GLuint skyboxVAO = sb.getVAO();

	//create frame buffer to render the shadow map
	ShadowMap directionalShadowmap = generateDirShadowRBuffer();

	//buffer and cubetexture for point shadows
	ShadowMap pointShadowmap = genPointShadowMap();

	//The world position of a global directional light and it's color
	glm::vec3 directionalLightDir(-0.2f, -1.0f, -0.9f);
	glm::vec3 directionalLightColor(0.2f, 0.2f, 0.2f);
	//
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f, 0.2f, 2.0f),
		glm::vec3(1.2f, 0.2f, 0.5f),
		glm::vec3(35.5f, 0.0f, -37.33f),
		glm::vec3(41.0f, 10.5f, 35.0f)
	};
	glm::vec3 pointLightColors[] = {
		glm::vec3(0.35f, 0.05f, 0.15f),
		glm::vec3(1.75f, 1.75f, 1.75f),
		glm::vec3(2.45f, 2.45f, 2.45f),
		glm::vec3(0.35f, 0.35f, 0.35f)
	};

	//To display lightsources (debug rendering of wireframe versions)
	std::vector<Model> primitives;
	for (GLuint i = 0; i < sizeof(pointLightPositions) / sizeof(glm::vec3); i++)
	{
		Model primitive(new RawPrimitive(dataArrays::rectanglevertices, sizeof(dataArrays::rectanglevertices), pointLightColors[i]));
		primitive.setScale(glm::vec3(0.25f, 0.25f, 0.25f));
		primitive.setRotation(glm::vec3(1.0f, 0.3f, 0.5f), 0.0f);

		primitives.push_back(primitive);
	}

	//texts rendering
	std::map<GLchar, Character> charMap;
	loadTTFont(charMap, "fonts/arial.ttf", 32);
	GLuint* textQuadVAOnVBO = generateFontQuad();

	GLdouble lastFrame = 0.0f;  	// Time of last frame
	//the game loop, that keeps on running until we tell GLFW to stop
	while (!glfwWindowShouldClose(window))
	{
		//Time calculations
		GLdouble currentTime = glfwGetTime();
		GLdouble deltaTime = currentTime - lastFrame;	// Time between current frame and last frame
		lastFrame = currentTime;

		//Main model rendering shader
		GLuint mainShader;
		mainShader = rendering::explodeMode ? theBlinnExplodeShader->getProgramId() : theBlinnShader->getProgramId();

		GLuint mainBatchShader;
		mainBatchShader = rendering::explodeMode ? theBlinnBatchExplodeShader->getProgramId() : theBlinnBatchShader->getProgramId();

		glfwSetWindowTitle(window, ("3D Scene Viewer [fps:" + std::to_string( (GLuint)ceil(1.0 / deltaTime) ) + "]").c_str());

		//Inputs
		if (Inputs::keys[GLFW_KEY_MINUS])
		{
			if (directionalLightColor.x > 0)
				directionalLightColor -= 0.01f;
			else
			{
				directionalLightColor.x = 0.0f;
				directionalLightColor.y = 0.0f;
				directionalLightColor.z= 0.0f;
			}
		}

		if (Inputs::keys[GLFW_KEY_EQUAL])
		{
			if (directionalLightColor.x < 1)
				directionalLightColor += 0.01f;
		}

		// Check and call events
		glfwPollEvents();

		clearScreen();

		//Calculate transformation matrices for our 3d space
		//projection matrix, perspective
		glm::mat4 projection = glm::perspective(theCamera->fov, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 500.0f);
		//view(eye) matrix
		theCamera->step(currentTime, deltaTime);
		glm::mat4 view = theCamera->getView();

		//Lights
		//Make some lights move around a bit				
		pointLightPositions[0].x = -(GLfloat)sin(currentTime) + 0.5f;
		pointLightPositions[0].y = -(GLfloat)cos(currentTime * 2) + 2.5f;
		pointLightPositions[1].x = -(GLfloat)sin(currentTime * 0.9f) * 1.25f + 0.5f;
		
		//for main shader (individual draw calls)
		setLightingParameters(mainShader, (GLfloat)currentTime, directionalLightColor, directionalLightDir, spotLightColor, theCamera->getCameraDirection(),
			theCamera->getPosition(), pointLightPositions, pointLightColors, sizeof(pointLightPositions) / sizeof(glm::vec3));
		//for batch rendering shader
		setLightingParameters(mainBatchShader, (GLfloat)currentTime, directionalLightColor, directionalLightDir, spotLightColor, theCamera->getCameraDirection(),
			theCamera->getPosition(), pointLightPositions, pointLightColors, sizeof(pointLightPositions) / sizeof(glm::vec3));

		//Render the scene, and use depth buffer to create a shadowmap - a texture with info about objects in shadow (from directional light)
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glCullFace(GL_FRONT);

		clearFrameBuffer(directionalShadowmap.shadowmapFBO);

		//Set all the space transformations for all the relvant shadow shaders
		glm::mat4 lightSpaceMatrix = setTransformationsForDirShadows(shadowMapShader->getProgramId(), directionalLightDir * (-26.0f));
		setTransformationsForDirShadows(shadowMapBatchShader->getProgramId(), directionalLightDir * (-26.0f));

		renderCalls(shadowMapShader->getProgramId(), shadowMapBatchShader->getProgramId(), outlineShader->getProgramId(), 
			outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(), models, primitives, pointLightPositions, directionalLightDir);

		//render point shadow for the first point light (same viewport dimensions, as the directional, but the new frame buffer)
		GLfloat pointLightFPlane = 0.0f;
		if (rendering::pointLightShadows)
		{
			clearFrameBuffer(pointShadowmap.shadowmapFBO);

			pointLightFPlane = setPointShadowTransforms(pointShadowMapShader->getProgramId(), pointLightPositions[0]);
			setPointShadowTransforms(pointShadowMapBatchShader->getProgramId(), pointLightPositions[0]);
			renderCalls(pointShadowMapShader->getProgramId(), pointShadowMapBatchShader->getProgramId(), outlineShader->getProgramId(),
				outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(), models, primitives, pointLightPositions, directionalLightDir);
		}		

		//Set matrices and "global" textures for general rendering shaders
		setShaderTransforms(mainShader, outlineShader->getProgramId(), theNormalsShader->getProgramId(),
			mainBatchShader, theBatchNormalsShader->getProgramId(), outlineBatchShader->getProgramId(),
			lightSourceShader->getProgramId(), skyBoxShader->getProgramId(), projection, view,
			lightSpaceMatrix, directionalShadowmap.shadowmapTexture, pointShadowmap.shadowmapTexture, pointLightFPlane, 
			thegBuffShader->getProgramId(), thegBuffBatchShader->getProgramId(), theDeferredtingShader->getProgramId(), theCamera->getPosition());

		//Render the scene to the frame buffer texture (normal rendering with lighting)
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glCullFace(GL_BACK);

		if (!rendering::deferredMode)	//straightforward rendering, isn't a pain in the ass to implement, and allows for anti-aliasing
		{
			blendingOn();

			clearFrameBuffer(multisampleFBO);

			renderCalls(mainShader, mainBatchShader, outlineShader->getProgramId(), outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(),
				models, primitives, pointLightPositions, directionalLightDir);

			renderText(textShader->getProgramId(), textQuadVAOnVBO[0], textQuadVAOnVBO[1], charMap,
				"I never asked for these " + std::to_string((GLuint)ceil(1.0 / deltaTime)) + " fps", 20.0f, 680.0f, glm::vec3(1.0f));

			renderSkybox(skyBoxShader->getProgramId(), skyboxVAO, cubeTexture);

			if (rendering::renderNormals)
				renderCalls(theNormalsShader->getProgramId(), theBatchNormalsShader->getProgramId(),
				outlineShader->getProgramId(), outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(),
				models, primitives, pointLightPositions, directionalLightDir);

			//Render the texture, containing the rendered scene to a full-screen quad
			blitMSampledScene(multisampleFBO, sceneFBO);
			blurSceneBrightnessTextr(gaussBlurShdr->getProgramId(), colorBufferTexture[1],
				renderingQuad, sizeof(dataArrays::quadVertices) / sizeof(GLfloat), pingpongFBOs, pingponTextures);
			renderFrameBufferToQuad(screenShaders[screenShaderId]->getProgramId(), colorBufferTexture[0], pingponTextures[1],
				renderingQuad, sizeof(dataArrays::quadVertices) / sizeof(GLfloat));
		}		
		else	//---G-BUFFER STUFF, deferred rendering, no anti-aliosing for you, slut
		{
			blendingOff();

			//activate gbuffer, fill it's textures with position, normals, color and lightspace data
			clearFrameBuffer(gBuffer);

			renderCalls(thegBuffShader->getProgramId(), thegBuffBatchShader->getProgramId(), outlineShader->getProgramId(), outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(),
				models, primitives, pointLightPositions, directionalLightDir);

			//render gbuffuer data to fullscreen quad, perform lighting/shadow calculations per-pixel, basically, 
			//instead of for all the fragments for all the vertices in the screen
			setLightingParameters(theDeferredtingShader->getProgramId(), (GLfloat)currentTime, directionalLightColor, directionalLightDir, spotLightColor, theCamera->getCameraDirection(),
				theCamera->getPosition(), pointLightPositions, pointLightColors, sizeof(pointLightPositions) / sizeof(glm::vec3));
			renderGBufferData(theDeferredtingShader->getProgramId(), gBufferTextures, sizeof(gBufferTextures) / sizeof(GLuint),
				renderingQuad, sizeof(dataArrays::quadVertices) / sizeof(GLfloat));

			blendingOn();
			//Blit the depth buffer from gBuffer to the default screen buffer, and straightforward-render some things (skybox, in this case)
			glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
			glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

			//just set, don't clear of course, because there is a scene, recreated from the G-buffer texture on top of the full-screen quad now
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			renderText(textShader->getProgramId(), textQuadVAOnVBO[0], textQuadVAOnVBO[1], charMap, 
				"I never asked for this " + std::to_string((GLuint)ceil(1.0 / deltaTime)) + " fps", 20.0f, 680.0f, glm::vec3(1.0f));

			renderSkybox(skyBoxShader->getProgramId(), skyboxVAO, cubeTexture);
		}
		//---END OF G-BUFFER STUFF

		if (rendering::rearView)
		{
			//Rotate 180 degrees (rear-view)
			theCamera->setPitch(theCamera->getPitch() + 180.0f);
			theCamera->setYaw(theCamera->getYaw() + 360.0f);
			view = theCamera->getView(true);

			setShaderTransforms(mainShader, outlineShader->getProgramId(), theNormalsShader->getProgramId(),
				mainBatchShader, theBatchNormalsShader->getProgramId(), outlineBatchShader->getProgramId(),
				lightSourceShader->getProgramId(), skyBoxShader->getProgramId(), projection, view, 
				lightSpaceMatrix, directionalShadowmap.shadowmapTexture, pointShadowmap.shadowmapTexture, pointLightFPlane,
				thegBuffShader->getProgramId(), outlineShader->getProgramId(), theDeferredtingShader->getProgramId(), theCamera->getPosition());

			clearFrameBuffer(multisampleFBO);

			renderCalls(mainShader, mainBatchShader, outlineShader->getProgramId(), outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(),
				models, primitives, pointLightPositions, directionalLightDir);

			renderSkybox(skyBoxShader->getProgramId(), skyboxVAO, cubeTexture);

			blitMSampledScene(multisampleFBO, sceneFBO);
			blurSceneBrightnessTextr(gaussBlurShdr->getProgramId(), colorBufferTexture[1], 
				renderingQuad, sizeof(dataArrays::quadVertices) / sizeof(GLfloat), pingpongFBOs, pingponTextures);
			renderFrameBufferToQuad(screenShaders[screenShaderId]->getProgramId(), colorBufferTexture[0],  pingponTextures[1],
				renderingMiniQuad, sizeof(dataArrays::cornerQuadVertices) / sizeof(GLfloat));

			//Reverse rear-view calculations (rear-view)
			theCamera->setPitch(theCamera->getPitch() - 180.0f);
			theCamera->setYaw(theCamera->getYaw() - 360.0f);
		}

		// Swap the buffers
		glfwSwapBuffers(window);

		Inputs::step(currentTime, deltaTime);
	}

	//When we're done with all framebuffer operations, do not forget to delete the framebuffer object
	glDeleteFramebuffers(1, &sceneFBO);
	glDeleteFramebuffers(1, &multisampleFBO);

	//As soon as we exit the game loop we would like to properly clean/delete all resources that were allocated
	glfwTerminate();

	return EXIT_SUCCESS;
}