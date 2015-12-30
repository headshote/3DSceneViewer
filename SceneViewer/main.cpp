#define GLEW_STATIC
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <SOIL/SOIL.h>
#include <assimp/Importer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering.h"
#include "dataArrays.h"
#include "DataStructures.hpp"

#include "Shader.h"
#include "Camera.h"
#include "Inputs.h"

#include "Renderable.h"
#include "Mesh.h"
#include "RawPrimitive.h"
#include "TextField.h"
#include "SkyBox.h"
#include "Model.h"

#include "FontFactory.h"
#include "SVFont.h"

#include "GlobalShaderVars.h"

#include "LightingSystem.h"

#include "ModelRenderingContext.h"
#include "SingleCallContext.h"
#include "BatchRenderContext.h"

using namespace renderables;
using namespace textandfonts;
using namespace shadervars;
using namespace lighting;
using namespace models;

const GLint SCREEN_WIDTH = 1280;
const GLint SCREEN_HEIGHT = 720;

const GLuint SHADOW_WIDTH = 1536;
const GLuint SHADOW_HEIGHT = 1536;

const GLint NUM_FRAGMENT_SAMPLES = 4;

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
	glfwSetKeyCallback(window, Inputs::glfwKeyCallback);

	glfwSetCursorPosCallback(window, Inputs::glfwMousePositionCallback);

	glfwSetScrollCallback(window, Inputs::glfwMouseScrollCallback);

	glfwSetMouseButtonCallback(window, Inputs::glfwMouseClicksCallback);

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
	glUniformMatrix4fv(glGetUniformLocation(shadowMapShader, "lightSpaceMatrix"), 1,
		GL_FALSE,	//transpose?
		glm::value_ptr(lightSpaceMatrix));

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
		glUniformMatrix4fv(glGetUniformLocation(pointShadowMapShader, (GLchar*)("lightSpaceTransforms[" + std::to_string(i) + "]").c_str()), 1,
		GL_FALSE,	//transpose?
		glm::value_ptr(lightSpaceMatrices[i]));
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

/**
* List of the models to load at the beginning of the program should be in the scene file
*/
std::vector<Model> loadModels()
{
	std::vector<Model> models;

	models.push_back(Model("models/nanosuit/nanosuit.obj"));
	GLuint grassTexture[] = { loadTexture("textures/grass.png", true, false), loadTexture("textures/mt_specular.png") };
	models.push_back(Model(new Mesh(dataArrays::transparentVertices, sizeof(dataArrays::transparentVertices), grassTexture, sizeof(grassTexture) / sizeof(GLuint)), "grass1"));
	GLuint glassTexture[] = { loadTexture("textures/blending_transparent_window.png", false, false), loadTexture("textures/mt_specular.png") };
	models.push_back(Model(new Mesh(dataArrays::transparentVertices, sizeof(dataArrays::transparentVertices), glassTexture, sizeof(glassTexture) / sizeof(GLuint), 0), "glass1"));
	models.push_back(Model("models/rock/rock.obj"));
	models.push_back(Model("models/planet/planet.obj"));
	//5
	GLuint metalTexture[] = { loadTexture("textures/metal1.jpg", false, false), loadTexture("textures/metal1_specular.png") };
	models.push_back(Model(new Mesh(dataArrays::planeVertices, sizeof(dataArrays::planeVertices), metalTexture, sizeof(metalTexture) / sizeof(GLuint)), "floor1"));
	//6
	GLuint brickTexture[] = { loadTexture("textures/brickwall.jpg", false, false), loadTexture("textures/mt_specular.png"), loadTexture("textures/brickwall_normal.jpg") };
	models.push_back(Model(new Mesh(dataArrays::wallVertices, sizeof(dataArrays::wallVertices), brickTexture, sizeof(brickTexture) / sizeof(GLuint)), "brick1"));
	//7
	GLuint brickPlainTexture[] = { loadTexture("textures/brickwall.jpg", false, false), loadTexture("textures/mt_specular.png") };
	models.push_back(Model(new Mesh(dataArrays::wallVertices, sizeof(dataArrays::wallVertices), brickPlainTexture, sizeof(brickPlainTexture) / sizeof(GLuint)), "brick2"));
	//8
	GLuint brickParallaxTexture[] = {
		loadTexture("textures/parallax_brix/bricks2.jpg", false, false),
		loadTexture("textures/parallax_brix/specular.png"),
		loadTexture("textures/parallax_brix/bricks2_normal.jpg"),
		loadTexture("textures/parallax_brix/bricks2_disp.jpg") };
	models.push_back(Model(new Mesh(dataArrays::wallVertices, sizeof(dataArrays::wallVertices), brickParallaxTexture, sizeof(brickParallaxTexture) / sizeof(GLuint)), "brick3"));
	//9
	GLuint tbParallaxTexture[] = {
		loadTexture("textures/woodbox/wood.png", false, false),
		loadTexture("textures/woodbox/specular.png"),
		loadTexture("textures/woodbox/toy_box_normal.png"),
		loadTexture("textures/woodbox/toy_box_disp.png") };
	models.push_back(Model(new Mesh(dataArrays::wallVertices, sizeof(dataArrays::wallVertices), tbParallaxTexture, sizeof(tbParallaxTexture) / sizeof(GLuint)), "wood1"));

	return models;
}

/**
*	Later on, contexts should be created from a scene file, for now, the whole scene is hardcoded like this
*/
std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>> createContexts(std::vector<Model>& models)
{
	std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>> modelContexts;

	for (GLuint i = 0; i < models.size(); ++i)
		modelContexts[models[i].getID()] = std::vector<std::shared_ptr<ModelRenderingContext>>();

	std::vector<glm::vec3> translations;
	std::vector<glm::vec3> scales(25 + 45 + 70, glm::vec3(0.33f));;
	std::vector<glm::vec3> rotationAxes;
	std::vector<GLfloat> rotationAngles;
	for (GLuint i = 0; i < 25; i++)
	{
		translations.push_back(glm::vec3(8.0f * sin(0.25f * (GLfloat)i), 0.0f, 8.0f * cos(0.25f * (GLfloat)i)));
		rotationAxes.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
		rotationAngles.push_back(45.0f + 2.0f * i);
	}
	for (GLuint i = 0; i < 45; i++)
	{
		translations.push_back(glm::vec3(20.0f * sin(0.14f * (GLfloat)i), 1.0f, 20.0f * cos(0.14f * (GLfloat)i)));
		rotationAxes.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
		rotationAngles.push_back(45.0f + 2.0f * i);
	}
	for (GLuint i = 0; i < 70; i++)
	{
		translations.push_back(glm::vec3(30.0f * sin(0.09f * (GLfloat)i), 2.0f, 30.0f * cos(0.09f * (GLfloat)i)));
		rotationAxes.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
		rotationAngles.push_back(45.0f + 2.0f * i);
	}
	modelContexts[models[0].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(1.0f, 0.0f, -2.0f), glm::vec3(0.33f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f)));
	modelContexts[models[0].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new BatchRenderContext(translations, scales, rotationAxes, rotationAngles)));

	std::vector<glm::vec3>().swap(translations);
	std::vector<glm::vec3>(25, glm::vec3(1.0f)).swap(scales);
	std::vector<glm::vec3>().swap(rotationAxes);
	std::vector<GLfloat>().swap(rotationAngles);
	for (GLuint i = 0; i < 25; i++)
	{
		translations.push_back(glm::vec3(15.75f * sin(0.25f * (GLfloat)i), 4.0f, 15.75f * cos(0.25f * (GLfloat)i)));
		rotationAxes.push_back(glm::vec3(0.5f, 0.5f, 0.0f));
		rotationAngles.push_back(30.0f + 45.0f * i);
	}
	modelContexts[models[3].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new BatchRenderContext(translations, scales, rotationAxes, rotationAngles)));

	std::vector<glm::vec3>().swap(translations);
	std::vector<glm::vec3>(5, glm::vec3(1.0f)).swap(scales);
	std::vector<glm::vec3>().swap(rotationAxes);
	std::vector<GLfloat>().swap(rotationAngles);
	for (GLuint i = 0; i < 5; i++)
	{
		translations.push_back(glm::vec3(12.75f * sin(5.25f * (GLfloat)i), 4.0f, 12.75f * cos(5.25f * (GLfloat)i)));
		rotationAxes.push_back(glm::vec3(0.5f, 0.5f, 0.0f));
		rotationAngles.push_back(45.0f + 2.0f * i);
	}
	modelContexts[models[4].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new BatchRenderContext(translations, scales, rotationAxes, rotationAngles)));

	modelContexts[models[1].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(-1.5f, 0.0f, -0.48f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts[models[2].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(0.5f, 0.0f, -0.48f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts[models[5].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts[models[6].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts[models[6].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(0.0f, 0.0f, 0.52f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false)));
	modelContexts[models[7].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(1.1f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts[models[7].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(1.1f, 0.0f, 0.52f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false)));
	modelContexts[models[8].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(-1.1f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts[models[8].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(-1.1f, 0.0f, 0.52f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false)));
	modelContexts[models[9].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(-2.1f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts[models[9].getID()].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(-2.1f, 0.0f, 0.52f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false)));

	//applies contexts to their models, because some context might have a lasting effect on the model's state
	//Like batch rendering context caches transforms, for batch rendering multiple model instances with one render calls, into VAO
	for (GLuint i = 0; i < models.size(); ++i)
	{
		Model& model = models[i];

		std::vector<std::shared_ptr<ModelRenderingContext>> currentMContexts = modelContexts[model.getID()];

		for (GLuint j = 0; j < currentMContexts.size(); ++j)
			currentMContexts[j]->applyContextStateToModel(model);
	}

	return modelContexts;
}

/**
*	Rendering the scene, as defined by it's models and their contexts (reuse the loaded models, just transform them and render whatever amount of times during the main rendering loop iteration)
*/
void renderCalls(const GLuint shaderProgram, const GLuint batchShaderProgram, const GLuint outlineShader, const GLuint outlineBatchShader, const GLuint lightSourceShader,
	std::vector<Model>& models, std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>>& modelContexts, std::vector<Model>& primitives, LightingSystem& lights)
{
	//Rendering models
	for (GLuint i = 0; i < models.size(); ++i)
	{
		Model& model = models[i];

		std::vector<std::shared_ptr<ModelRenderingContext>> currentMContexts = modelContexts[model.getID()];

		for (GLuint j = 0; j < currentMContexts.size(); ++j)
			currentMContexts[j]->doRendering(model, shaderProgram, batchShaderProgram, outlineShader, outlineBatchShader);
	}

	//Render lightsources (debug primitives at the places of point lights)
	GLuint i = 0;
	for (auto iterator = primitives.begin(); iterator != primitives.end(); i++, iterator++)
	{
		iterator->setTranslation(lights.getPointLight(i).position);
		iterator->drawCall(lightSourceShader);
	}
	//Directional light "origin" far, far away
	primitives[0].setTranslation(lights.getDirLight().direction * (-26.0f));
	primitives[0].drawCall(lightSourceShader);
}

int main()
{
	GLFWwindow* window = setUpWindow(SCREEN_WIDTH, SCREEN_HEIGHT);
	if (window == nullptr)
		return EXIT_FAILURE;

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
	std::vector<Model> models = loadModels();
	
	//LOad the contexts for the models (because the same model can be rendered multiple times with different transforms during the frame)
	std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>> modelContexts = createContexts(models);
	
	//Cube map
	SkyBox skyBox(std::vector<std::string> {"textures/cubemap/mnight_rt.jpg",  "textures/cubemap/mnight_lf.jpg",
		"textures/cubemap/mnight_up.jpg", "textures/cubemap/mnight_dn.jpg", 
		"textures/cubemap/mnight_bk.jpg", "textures/cubemap/mnight_ft.jpg" });

	//create frame buffer to render the shadow map
	ShadowMap directionalShadowmap = generateDirShadowRBuffer();

	//buffer and cubetexture for point shadows
	ShadowMap pointShadowmap = genPointShadowMap();

	//texts rendering
	std::shared_ptr<TextField> textField1 = FontFactory::instance()->CreateRenderableText("fonts/arial.ttf", 32, SCREEN_WIDTH, SCREEN_HEIGHT, "I never asked fo this");

	//Shader vars, that are shared among many/most of the sahders are all registered here, and are updated for all the shaders in one call
	GlobalShaderVars::instance()->addMat4Var("projection");
	GlobalShaderVars::instance()->addMat4Var("view");
	GlobalShaderVars::instance()->addMat4Var("lightSpaceMatrix");
	GlobalShaderVars::instance()->addMat4Var("pinnedView");

	GlobalShaderVars::instance()->addTextureVar("shadowMap");
	GlobalShaderVars::instance()->addTextureVar("pointShadowMap");
	GlobalShaderVars::instance()->addFloatVar("pointLightFarPlane");
	GlobalShaderVars::instance()->addBoolVar("pointLightShadows");

	GlobalShaderVars::instance()->addVec3Var("viewerPos");

	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnExplodeShader->getProgramId(), "projection");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnExplodeShader->getProgramId(), "view");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnExplodeShader->getProgramId(), "lightSpaceMatrix");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnExplodeShader->getProgramId(), "shadowMap");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnExplodeShader->getProgramId(), "pointShadowMap");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnExplodeShader->getProgramId(), "pointLightFarPlane");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnExplodeShader->getProgramId(), "pointLightShadows");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnExplodeShader->getProgramId(), "viewerPos");

	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnShader->getProgramId(), "projection");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnShader->getProgramId(), "view");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnShader->getProgramId(), "lightSpaceMatrix");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnShader->getProgramId(), "shadowMap");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnShader->getProgramId(), "pointShadowMap");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnShader->getProgramId(), "pointLightFarPlane");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnShader->getProgramId(), "pointLightShadows");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnShader->getProgramId(), "viewerPos");

	GlobalShaderVars::instance()->subscribeShaderToVar(outlineShader->getProgramId(), "projection");
	GlobalShaderVars::instance()->subscribeShaderToVar(outlineShader->getProgramId(), "view");

	GlobalShaderVars::instance()->subscribeShaderToVar(theNormalsShader->getProgramId(), "projection");
	GlobalShaderVars::instance()->subscribeShaderToVar(theNormalsShader->getProgramId(), "view");

	GlobalShaderVars::instance()->subscribeShaderToVar(lightSourceShader->getProgramId(), "projection");
	GlobalShaderVars::instance()->subscribeShaderToVar(lightSourceShader->getProgramId(), "view");

	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchExplodeShader->getProgramId(), "projection");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchExplodeShader->getProgramId(), "view");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchExplodeShader->getProgramId(), "lightSpaceMatrix");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchExplodeShader->getProgramId(), "shadowMap");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchExplodeShader->getProgramId(), "pointShadowMap");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchExplodeShader->getProgramId(), "pointLightFarPlane");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchExplodeShader->getProgramId(), "pointLightShadows");

	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchShader->getProgramId(), "projection");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchShader->getProgramId(), "view");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchShader->getProgramId(), "lightSpaceMatrix");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchShader->getProgramId(), "shadowMap");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchShader->getProgramId(), "pointShadowMap");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchShader->getProgramId(), "pointLightFarPlane");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchShader->getProgramId(), "pointLightShadows");

	GlobalShaderVars::instance()->subscribeShaderToVar(theBatchNormalsShader->getProgramId(), "projection");
	GlobalShaderVars::instance()->subscribeShaderToVar(theBatchNormalsShader->getProgramId(), "view");

	GlobalShaderVars::instance()->subscribeShaderToVar(outlineBatchShader->getProgramId(), "projection");
	GlobalShaderVars::instance()->subscribeShaderToVar(outlineBatchShader->getProgramId(), "view");

	GlobalShaderVars::instance()->subscribeShaderToVar(thegBuffShader->getProgramId(), "projection");
	GlobalShaderVars::instance()->subscribeShaderToVar(thegBuffShader->getProgramId(), "view");
	GlobalShaderVars::instance()->subscribeShaderToVar(thegBuffShader->getProgramId(), "lightSpaceMatrix");
	GlobalShaderVars::instance()->subscribeShaderToVar(thegBuffShader->getProgramId(), "viewerPos");

	GlobalShaderVars::instance()->subscribeShaderToVar(thegBuffBatchShader->getProgramId(), "projection");
	GlobalShaderVars::instance()->subscribeShaderToVar(thegBuffBatchShader->getProgramId(), "view");
	GlobalShaderVars::instance()->subscribeShaderToVar(thegBuffBatchShader->getProgramId(), "lightSpaceMatrix");
	GlobalShaderVars::instance()->subscribeShaderToVar(thegBuffBatchShader->getProgramId(), "viewerPos");

	GlobalShaderVars::instance()->subscribeShaderToVar(theDeferredtingShader->getProgramId(), "shadowMap");
	GlobalShaderVars::instance()->subscribeShaderToVar(theDeferredtingShader->getProgramId(), "pointShadowMap");
	GlobalShaderVars::instance()->subscribeShaderToVar(theDeferredtingShader->getProgramId(), "pointLightFarPlane");
	GlobalShaderVars::instance()->subscribeShaderToVar(theDeferredtingShader->getProgramId(), "pointLightShadows");
	GlobalShaderVars::instance()->subscribeShaderToVar(theDeferredtingShader->getProgramId(), "viewerPos");

	GlobalShaderVars::instance()->subscribeShaderToVar(skyBoxShader->getProgramId(), "projection");
	GlobalShaderVars::instance()->subscribeShaderToVar(skyBoxShader->getProgramId(), "pinnedView");

	//To display lightsources (debug rendering of wireframe quads in the places, where the pointlights are)
	//Lights
	LightingSystem lights;

	std::vector<Model> primitives;
	for (GLuint i = 0; i < lights.getNumPointLights(); i++)
	{
		Model primitive(new RawPrimitive( dataArrays::rectanglevertices, sizeof(dataArrays::rectanglevertices), lights.getPointLight(i).color ), "lghtsrc" + i);
		primitive.setScale(glm::vec3(0.25f, 0.25f, 0.25f));
		primitive.setRotation(glm::vec3(1.0f, 0.3f, 0.5f), 0.0f);

		primitives.push_back(primitive);
	}

	GLdouble lastFrame = 0.0f;  	// Time of last frame
	//the game loop, that keeps on running until we tell GLFW to stop
	while (!glfwWindowShouldClose(window))
	{
		//Time calculations
		GLdouble currentTime = glfwGetTime();
		GLdouble deltaTime = currentTime - lastFrame;	// Time between current frame and last frame
		lastFrame = currentTime;

		// Check and call glfw window events
		glfwPollEvents();

		//Main model rendering shader
		GLuint mainShader = rendering::explodeMode ? theBlinnExplodeShader->getProgramId() : theBlinnShader->getProgramId();

		GLuint mainBatchShader = rendering::explodeMode ? theBlinnBatchExplodeShader->getProgramId() : theBlinnBatchShader->getProgramId();

		glfwSetWindowTitle(window, ("3D Scene Viewer [fps:" + std::to_string( (GLuint)ceil(1.0 / deltaTime) ) + "]").c_str());

		//----Inputs
		//---One-time ations, right upon the key release
		if (Inputs::instance()->isKeyReleased(GLFW_KEY_ESCAPE)) //Close window
			glfwSetWindowShouldClose(window, GL_TRUE);

		if (Inputs::instance()->isKeyReleased(GLFW_KEY_ENTER)) 	//Switch between wireframe and fill mode
		{
			GLint polygonMode;
			glGetIntegerv(GL_POLYGON_MODE, &polygonMode);

			glPolygonMode(GL_FRONT_AND_BACK, polygonMode == GL_FILL ? GL_LINE : GL_FILL);
		}

		for (GLuint numKey = GLFW_KEY_1; numKey <= GLFW_KEY_8; ++numKey) //Switch between post-processing filters
			if (Inputs::instance()->isKeyReleased(numKey))
				rendering::screenShaderId = numKey - GLFW_KEY_1;

		if (Inputs::instance()->isKeyReleased(GLFW_KEY_R))
			rendering::rearView = !rendering::rearView;

		if (Inputs::instance()->isKeyReleased(GLFW_KEY_Q)) //Model outlines on/off
		{
			rendering::highlightModels = !rendering::highlightModels;
			rendering::explodeMode = false;
			rendering::dotMode = false;
			rendering::renderNormals = false;
		}

		if (Inputs::instance()->isKeyReleased(GLFW_KEY_G))
		{
			rendering::dotMode = !rendering::dotMode;
			rendering::explodeMode = false;
			rendering::highlightModels = false;
			rendering::renderNormals = false;
		}

		if (Inputs::instance()->isKeyReleased(GLFW_KEY_X))
		{
			rendering::explodeMode = !rendering::explodeMode;
			rendering::dotMode = false;
			rendering::highlightModels = false;
			rendering::renderNormals = false;
		}

		if (Inputs::instance()->isKeyReleased(GLFW_KEY_Z))
		{
			rendering::renderNormals = !rendering::renderNormals;
			rendering::dotMode = false;
			rendering::highlightModels = false;
			rendering::explodeMode = false;
		}

		if (Inputs::instance()->isKeyReleased(GLFW_KEY_V))
			rendering::pointLightShadows = !rendering::pointLightShadows;

		if (Inputs::instance()->isKeyReleased(GLFW_KEY_C))
		{
			rendering::deferredMode = !rendering::deferredMode;
			std::cout << (rendering::deferredMode ? "Entering deferred rendering mode" : "Entering straightforward rendering mode") << std::endl;
		}

		if (Inputs::instance()->isKeyReleased(GLFW_KEY_F))
		{
			GLfloat clorValue = lights.getSpotLight().color.x == 0.0f ? 0.5f : 0.0f;
			lights.setSpotLightColor(glm::vec3(clorValue));
		}
		
		//---continious actions
		if (Inputs::instance()->isKeyHeld(GLFW_KEY_LEFT_BRACKET))
		{
			rendering::hdrExposure -= 0.25f;
			std::cout << "hdr exposure: " << rendering::hdrExposure << std::endl;
		}
		if (Inputs::instance()->isKeyHeld(GLFW_KEY_RIGHT_BRACKET))
		{
			rendering::hdrExposure += 0.25f;
			std::cout << "hdr exposure: " << rendering::hdrExposure << std::endl;
		}
		
		if (Inputs::instance()->isKeyHeld(GLFW_KEY_MINUS))
		{
			const glm::vec3& directionalLightColor = lights.getDirLight().color;
			if (directionalLightColor.x > 0)
				lights.setDirLightColor(glm::vec3(directionalLightColor - 0.01f));
			else
				lights.setDirLightColor(glm::vec3(0.0f));
		}

		if (Inputs::instance()->isKeyHeld(GLFW_KEY_EQUAL))
		{
			const glm::vec3& directionalLightColor = lights.getDirLight().color;
			if (directionalLightColor.x < 1)
				lights.setDirLightColor(glm::vec3(directionalLightColor + 0.01f));
		}

		//Clearing whatever was rendered during the previous frame
		clearScreen();

		//Calculate transformation matrices for our 3d space
		//projection matrix, perspective
		glm::mat4 projection = glm::perspective(theCamera->fov, (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 500.0f);
		//view(eye) matrix
		theCamera->step(currentTime, deltaTime);
		glm::mat4 view = theCamera->getView();

		//Make some lights move around a bit		
		lights.setPointightPosition(0, glm::vec3(-(GLfloat)sin(currentTime) + 0.5f, -(GLfloat)cos(currentTime * 2) + 2.5f, 2.0f));
		lights.setPointightPosition(1, glm::vec3(-(GLfloat)sin(currentTime * 0.9f) * 1.25f + 50.5f, 0.2f, 0.5f));

		//make some models move around a bit
		std::shared_ptr<ModelRenderingContext> ourHeroOurHero = modelContexts[models[0].getID()][0];
		ourHeroOurHero->setTranslation(glm::vec3(5.25f * sin(0.5f * (GLfloat)glfwGetTime()), 0.0f, 5.25f * cos(0.5f * (GLfloat)glfwGetTime())), 0);
		ourHeroOurHero->setRotation(glm::vec3(0.0f, 1.0f, 0.0f), 45.0f + 90.0f * (GLfloat)glfwGetTime(), 0);
		
		//Render the scene, and use depth buffer to create a shadowmap - a texture with info about objects in shadow (from directional light)
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glCullFace(GL_FRONT);

		clearFrameBuffer(directionalShadowmap.shadowmapFBO);

		//Set all the space transformations for all the relvant shadow shaders
		glm::mat4 lightSpaceMatrix = setTransformationsForDirShadows(shadowMapShader->getProgramId(), lights.getDirLight().direction * (-26.0f));
		setTransformationsForDirShadows(shadowMapBatchShader->getProgramId(), lights.getDirLight().direction * (-26.0f));

		renderCalls(shadowMapShader->getProgramId(), shadowMapBatchShader->getProgramId(), outlineShader->getProgramId(), 
			outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(), models, modelContexts, primitives, lights);

		//render point shadow for the first point light (same viewport dimensions, as the directional, but the new frame buffer)
		GLfloat pointLightFPlane = 0.0f;
		if (rendering::pointLightShadows)
		{
			clearFrameBuffer(pointShadowmap.shadowmapFBO);

			pointLightFPlane = setPointShadowTransforms(pointShadowMapShader->getProgramId(), lights.getPointLight(0).position);
			setPointShadowTransforms(pointShadowMapBatchShader->getProgramId(), lights.getPointLight(0).position);
			renderCalls(pointShadowMapShader->getProgramId(), pointShadowMapBatchShader->getProgramId(), outlineShader->getProgramId(),
				outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(), models, modelContexts, primitives, lights);
		}		

		//Lights
		lights.setCameraDirection(theCamera->getCameraDirection());
		lights.setCameraPosition(theCamera->getPosition());
		lights.setLightingParameters(mainShader);
		lights.setLightingParameters(mainBatchShader);

		if (rendering::explodeMode)
		{
			glUniform1f(glGetUniformLocation(mainShader, "time"), (GLfloat)currentTime);
			glUniform1f(glGetUniformLocation(mainBatchShader, "time"), (GLfloat)currentTime);
		}

		GlobalShaderVars::instance()->setMat4Var("projection", projection);
		GlobalShaderVars::instance()->setMat4Var("view", view);
		GlobalShaderVars::instance()->setMat4Var("pinnedView", glm::mat4(glm::mat3(view)));
		GlobalShaderVars::instance()->setMat4Var("lightSpaceMatrix", lightSpaceMatrix);
		GlobalShaderVars::instance()->setTextureVar("shadowMap", 16, GL_TEXTURE_2D, directionalShadowmap.shadowmapTexture);
		GlobalShaderVars::instance()->setTextureVar("pointShadowMap", 17, GL_TEXTURE_CUBE_MAP, pointShadowmap.shadowmapTexture);
		GlobalShaderVars::instance()->setFloatVar("pointLightFarPlane", pointLightFPlane);
		GlobalShaderVars::instance()->setBoolVar("pointLightShadows", rendering::pointLightShadows);
		GlobalShaderVars::instance()->setVec3Var("viewerPos", theCamera->getPosition());

		GlobalShaderVars::instance()->updateAllVars();

		//Render the scene to the frame buffer texture (normal rendering with lighting)
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glCullFace(GL_BACK);

		if (!rendering::deferredMode)	//straightforward rendering, isn't a pain in the ass to implement, and allows for anti-aliasing
		{
			blendingOn();

			clearFrameBuffer(multisampleFBO);

			renderCalls(mainShader, mainBatchShader, outlineShader->getProgramId(), outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(),
				models, modelContexts, primitives, lights);

			textField1->setText("I never asked for these " + std::to_string((GLuint)ceil(1.0 / deltaTime)) + " fps");
			textField1->setPosition(20.0f, 680.0f);
			textField1->drawCall(textShader->getProgramId());

			skyBox.drawCall(skyBoxShader->getProgramId());

			if (rendering::renderNormals)
				renderCalls(theNormalsShader->getProgramId(), theBatchNormalsShader->getProgramId(),
				outlineShader->getProgramId(), outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(),
				models, modelContexts, primitives, lights);

			//Render the texture, containing the rendered scene to a full-screen quad
			blitMSampledScene(multisampleFBO, sceneFBO);
			blurSceneBrightnessTextr(gaussBlurShdr->getProgramId(), colorBufferTexture[1],
				renderingQuad, sizeof(dataArrays::quadVertices) / sizeof(GLfloat), pingpongFBOs, pingponTextures);
			renderFrameBufferToQuad(screenShaders[rendering::screenShaderId]->getProgramId(), colorBufferTexture[0], pingponTextures[1],
				renderingQuad, sizeof(dataArrays::quadVertices) / sizeof(GLfloat));
		}		
		else	//---G-BUFFER STUFF, deferred rendering, no anti-aliosing for you, slut
		{
			blendingOff();

			//activate gbuffer, fill it's textures with position, normals, color and lightspace data
			clearFrameBuffer(gBuffer);

			renderCalls(thegBuffShader->getProgramId(), thegBuffBatchShader->getProgramId(), outlineShader->getProgramId(), outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(),
				models, modelContexts, primitives, lights);

			//render gbuffuer data to fullscreen quad, perform lighting/shadow calculations per-pixel, basically, 
			//instead of for all the fragments for all the vertices in the screen
			lights.setLightingParameters(theDeferredtingShader->getProgramId());
			if (rendering::explodeMode)
				glUniform1f(glGetUniformLocation(theDeferredtingShader->getProgramId(), "time"), (GLfloat)currentTime);
			renderGBufferData(theDeferredtingShader->getProgramId(), gBufferTextures, sizeof(gBufferTextures) / sizeof(GLuint),
				renderingQuad, sizeof(dataArrays::quadVertices) / sizeof(GLfloat));

			blendingOn();
			//Blit the depth buffer from gBuffer to the default screen buffer, and straightforward-render some things (skybox, in this case)
			glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Write to default framebuffer
			glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

			//just set, don't clear of course, because there is a scene, recreated from the G-buffer texture on top of the full-screen quad now
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			textField1->setText("I never asked for these " + std::to_string((GLuint)ceil(1.0 / deltaTime)) + " fps");
			textField1->setPosition(20.0f, 680.0f);
			textField1->drawCall(textShader->getProgramId());

			skyBox.drawCall(skyBoxShader->getProgramId());
		}
		//---END OF G-BUFFER STUFF

		if (rendering::rearView)
		{
			//Rotate 180 degrees (rear-view)
			theCamera->setPitch(theCamera->getPitch() + 180.0f);
			theCamera->setYaw(theCamera->getYaw() + 360.0f);
			view = theCamera->getView(true);

			GlobalShaderVars::instance()->setMat4Var("view", view);
			GlobalShaderVars::instance()->setMat4Var("pinnedView", glm::mat4(glm::mat3(view)));

			GlobalShaderVars::instance()->updateAllVars();

			clearFrameBuffer(multisampleFBO);

			renderCalls(mainShader, mainBatchShader, outlineShader->getProgramId(), outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(),
				models, modelContexts, primitives, lights);

			skyBox.drawCall(skyBoxShader->getProgramId());

			blitMSampledScene(multisampleFBO, sceneFBO);
			blurSceneBrightnessTextr(gaussBlurShdr->getProgramId(), colorBufferTexture[1], 
				renderingQuad, sizeof(dataArrays::quadVertices) / sizeof(GLfloat), pingpongFBOs, pingponTextures);
			renderFrameBufferToQuad(screenShaders[rendering::screenShaderId]->getProgramId(), colorBufferTexture[0], pingponTextures[1],
				renderingMiniQuad, sizeof(dataArrays::cornerQuadVertices) / sizeof(GLfloat));

			//Reverse rear-view calculations (rear-view)
			theCamera->setPitch(theCamera->getPitch() - 180.0f);
			theCamera->setYaw(theCamera->getYaw() - 360.0f);
		}

		// Swap the buffers
		glfwSwapBuffers(window);

		Inputs::instance()->step(currentTime, deltaTime);
	}

	//When we're done with all framebuffer operations, do not forget to delete the framebuffer object
	glDeleteFramebuffers(1, &sceneFBO);
	glDeleteFramebuffers(1, &multisampleFBO);
	glDeleteFramebuffers(sizeof(pingpongFBOs) / sizeof(GLuint), pingpongFBOs);
	glDeleteFramebuffers(1, &gBuffer);

	//As soon as we exit the game loop we would like to properly clean/delete all resources that were allocated
	glfwTerminate();

	return EXIT_SUCCESS;
}