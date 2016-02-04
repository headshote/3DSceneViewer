#define GLEW_STATIC
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>

#include <thread>

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

#include "DirectionalShadowMap.h"
#include "PointShadowMap.h"

#include "MultisampledBlurFB.h"
#include "GBuffer.h"

#include "AsyncModelLoader.h"

using namespace renderables;
using namespace textandfonts;
using namespace shadervars;
using namespace lighting;
using namespace models;
using namespace shadows;
using namespace framebuffers;

GLuint screenWidth = 1280;
GLuint screenHeight = 720;

GLuint shadowViewWidth = 1536;
GLuint shadowViewHeight = 1536;

GLint nFragmentSamples = 4;

GLFWwindow* setUpWindow(int width, int height, GLFWwindow* threadWin)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GL_TRUE);

	//Next we're required to create a window object.
	GLFWwindow* window = glfwCreateWindow(width, height, "Scene Viewer", nullptr, threadWin);
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
	//glfwWindowHint(GLFW_SAMPLES, nFragmentSamples);	//would have been useful, if we rendered directly to the window's default frame buffer
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
*	Later on, contexts should be created from a scene file, for now, the whole scene is hardcoded like this
*/
void createContexts(std::vector<Model>* mdels, std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>>* mdelContexts)
{
	std::vector<Model>& models = *mdels;
	std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>>& modelContexts = *mdelContexts;

	modelContexts["models/nanosuit/nanosuit.obj"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	modelContexts["grass1"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	modelContexts["glass1"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	modelContexts["models/rock/rock.obj"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	modelContexts["models/planet/planet.obj"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	modelContexts["floor1"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	modelContexts["brick1"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	modelContexts["brick2"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	modelContexts["brick3"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	modelContexts["wood1"] = std::vector<std::shared_ptr<ModelRenderingContext>>();

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
	modelContexts["models/nanosuit/nanosuit.obj"].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(1.0f, 0.0f, -2.0f), glm::vec3(0.33f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f)));
	modelContexts["models/nanosuit/nanosuit.obj"].push_back(std::shared_ptr<ModelRenderingContext>(new BatchRenderContext(translations, scales, rotationAxes, rotationAngles)));

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
	modelContexts["models/rock/rock.obj"].push_back(std::shared_ptr<ModelRenderingContext>(new BatchRenderContext(translations, scales, rotationAxes, rotationAngles)));

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
	modelContexts["models/planet/planet.obj"].push_back(std::shared_ptr<ModelRenderingContext>(new BatchRenderContext(translations, scales, rotationAxes, rotationAngles)));

	modelContexts["grass1"].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(-1.5f, 0.0f, -0.48f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["glass1"].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(0.5f, 0.0f, -0.48f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["floor1"].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["brick1"].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["brick1"].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(0.0f, 0.0f, 0.52f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false)));
	modelContexts["brick2"].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(1.1f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["brick2"].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(1.1f, 0.0f, 0.52f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false)));
	modelContexts["brick3"].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(-1.1f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["brick3"].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(-1.1f, 0.0f, 0.52f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false)));
	modelContexts["wood1"].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(-2.1f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["wood1"].push_back(std::shared_ptr<ModelRenderingContext>(new SingleCallContext(glm::vec3(-2.1f, 0.0f, 0.52f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false)));

	//applies contexts to their models, because some context might have a lasting effect on the model's state
	//Like batch rendering context caches transforms, for batch rendering multiple model instances with one render calls, into VAO
	for (GLuint i = 0; i < models.size(); ++i)
	{
		Model& model = models[i];

		std::vector<std::shared_ptr<ModelRenderingContext>> currentMContexts = modelContexts[model.getID()];

		for (GLuint j = 0; j < currentMContexts.size(); ++j)
			currentMContexts[j]->applyContextStateToModel(model);
	}
}

/**
* List of the models to load at the beginning of the program should be in the scene file
*/
void loadModels(std::vector<Model>* mdels, std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>>* mdelContexts, std::vector<AsyncData>* modelQueue, GLFWwindow* window)
{
	if (nullptr != window)
	{
		glfwMakeContextCurrent(window);
		glewExperimental = GL_TRUE;
		glewInit();
	}

	std::vector<Model>& models = *mdels;

	//0
	std::string nanoPath = "models/nanosuit/nanosuit.obj";
	AsyncModelLoader::instance()->loadModel(&nanoPath, modelQueue);
	//models.push_back(Model("models/nanosuit/nanosuit.obj"));

	//1
	GLuint grassTexture[] = { loadTexture("textures/grass.png", true, false), loadTexture("textures/mt_specular.png") };
	models.push_back(Model(new Mesh(dataArrays::transparentVertices, sizeof(dataArrays::transparentVertices), grassTexture, sizeof(grassTexture) / sizeof(GLuint)), "grass1"));

	//2
	GLuint glassTexture[] = { loadTexture("textures/blending_transparent_window.png", false, false), loadTexture("textures/mt_specular.png") };
	models.push_back(Model(new Mesh(dataArrays::transparentVertices, sizeof(dataArrays::transparentVertices), glassTexture, sizeof(glassTexture) / sizeof(GLuint), 0), "glass1"));

	//3
	std::string rockPath = "models/rock/rock.obj";
	AsyncModelLoader::instance()->loadModel(&rockPath, modelQueue);

	////4
	std::string planetPath = "models/planet/planet.obj";
	AsyncModelLoader::instance()->loadModel(&planetPath, modelQueue);

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

	createContexts(mdels, mdelContexts);

	glFlush();
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

void scriptedMovements(std::vector<Model>& models, std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>>& modelContexts)
{
	if (models.size() > 0)
	{
		auto contextIter = modelContexts.find("models/nanosuit/nanosuit.obj");
		if (contextIter != modelContexts.cend() && modelContexts["models/nanosuit/nanosuit.obj"].size() > 0)
		{
			std::shared_ptr<ModelRenderingContext> ourHeroOurHero = modelContexts["models/nanosuit/nanosuit.obj"][0];
			ourHeroOurHero->setTranslation(glm::vec3(5.25f * sin(0.5f * (GLfloat)glfwGetTime()), 0.0f, 5.25f * cos(0.5f * (GLfloat)glfwGetTime())), 0);
			ourHeroOurHero->setRotation(glm::vec3(0.0f, 1.0f, 0.0f), 45.0f + 90.0f * (GLfloat)glfwGetTime(), 0);
		}
	}
}

void checkLoadedModels(std::vector<Model>& models, std::vector<AsyncData>& modelQueue, std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>>& modelContexts)
{
	while (modelQueue.size() > 0)
	{
		AsyncData& vertexData = modelQueue[modelQueue.size()-1];
		Model mdl1(vertexData);
		modelQueue.pop_back();

		//applies contexts to their models, because some context might have a lasting effect on the model's state
		//Like batch rendering context caches transforms, for batch rendering multiple model instances with one render calls, into VAO		
		std::vector<std::shared_ptr<ModelRenderingContext>> currentMContexts = modelContexts[mdl1.getID()];

		for (GLuint j = 0; j < currentMContexts.size(); ++j)
			currentMContexts[j]->applyContextStateToModel(mdl1);

		models.push_back(mdl1);
	}
}

int main()
{
	glfwInit();

	GLFWwindow* window = setUpWindow(screenWidth, screenHeight, nullptr);
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
	MultisampledBlurFB msFBO(screenWidth, screenHeight, nFragmentSamples);

	//G-buffer, for deferred rendering
	GBuffer gBuff(screenWidth, screenHeight);

	//This vao, contains the quad in NDC, which will have the FBO texture applied to it
	RawPrimitive rq(dataArrays::quadVertices, sizeof(dataArrays::quadVertices));
	GLuint renderingQuad = rq.getVAO();
	RawPrimitive rmq(dataArrays::cornerQuadVertices, sizeof(dataArrays::cornerQuadVertices));
	GLuint renderingMiniQuad = rmq.getVAO();

	//Load the models and the contexts for the models (because the same model can be rendered multiple times with different transforms during the frame)
	std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>> modelContexts;
	std::vector<Model> models;
	std::vector<AsyncData> modelQueue;
	loadModels(&models, &modelContexts, &modelQueue, nullptr);
	
	//Cube map
	SkyBox skyBox(std::vector<std::string> {"textures/cubemap/mnight_rt.jpg",  "textures/cubemap/mnight_lf.jpg",
		"textures/cubemap/mnight_up.jpg", "textures/cubemap/mnight_dn.jpg", 
		"textures/cubemap/mnight_bk.jpg", "textures/cubemap/mnight_ft.jpg" });

	//create frame buffer to render the shadow map
	DirectionalShadowMap dirShadow(shadowViewWidth, shadowViewHeight);

	//buffer and cubetexture for point shadows
	PointShadowMap pointShadow(shadowViewWidth, shadowViewHeight);

	//texts rendering
	std::shared_ptr<TextField> textField1 = FontFactory::instance()->CreateRenderableText("fonts/arial.ttf", 32, screenWidth, screenHeight, "I never asked fo this");

	//Shader vars, that are shared among many/most of the sahders are all registered here, and are updated for all the shaders in one call
	GlobalShaderVars::instance()->addMat4Var("projection");
	GlobalShaderVars::instance()->addMat4Var("view");
	GlobalShaderVars::instance()->addMat4Var("lightSpaceMatrix");
	GlobalShaderVars::instance()->addMat4Var("pinnedView");

	GlobalShaderVars::instance()->addTextureVar("shadowMap");
	GlobalShaderVars::instance()->addTextureVar("pointShadowMap");
	GlobalShaderVars::instance()->addFloatVar("pointLightFarPlane");
	GlobalShaderVars::instance()->addFloatVar("time"); 
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
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnExplodeShader->getProgramId(), "time");

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
	GlobalShaderVars::instance()->subscribeShaderToVar(theBlinnBatchExplodeShader->getProgramId(), "time");

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
	GlobalShaderVars::instance()->subscribeShaderToVar(theDeferredtingShader->getProgramId(), "time");

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

		if (AsyncModelLoader::instance()->isDone())
			checkLoadedModels(models, modelQueue, modelContexts);

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
		if (Inputs::instance()->isKeyPressed(GLFW_KEY_LEFT_BRACKET))
		{
			rendering::hdrExposure -= 0.25f;
			std::cout << "hdr exposure: " << rendering::hdrExposure << std::endl;
		}
		if (Inputs::instance()->isKeyPressed(GLFW_KEY_RIGHT_BRACKET))
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
		glm::mat4 projection = glm::perspective(theCamera->fov, (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 500.0f);
		//view(eye) matrix
		theCamera->step(currentTime, deltaTime);
		glm::mat4 view = theCamera->getView();

		//Make some lights move around a bit		
		lights.setPointightPosition(0, glm::vec3(-(GLfloat)sin(currentTime) + 0.5f, -(GLfloat)cos(currentTime * 2) + 2.5f, 2.0f));
		lights.setPointightPosition(1, glm::vec3(-(GLfloat)sin(currentTime * 0.9f) * 1.25f + 50.5f, 0.2f, 0.5f));

		//make some models move around a bit
		scriptedMovements(models, modelContexts);

		//Render the scene, and use depth buffer to create a shadowmap - a texture with info about objects in shadow (from directional light)		
		dirShadow.renderShadowMap(shadowMapShader->getProgramId(), shadowMapBatchShader->getProgramId(), lights.getDirLight().direction * (-26.0f), models, modelContexts);

		//render point shadow for the first point light (same viewport dimensions, as the directional, but the new frame buffer)		
		if (rendering::pointLightShadows)
			pointShadow.renderShadowMap(pointShadowMapShader->getProgramId(), pointShadowMapBatchShader->getProgramId(), lights.getPointLight(0).position, models, modelContexts);			

		//Lights
		lights.setCameraDirection(theCamera->getCameraDirection());
		lights.setCameraPosition(theCamera->getPosition());
		lights.setLightingParameters(mainShader);
		lights.setLightingParameters(mainBatchShader);

		GlobalShaderVars::instance()->setMat4Var("projection", projection);
		GlobalShaderVars::instance()->setMat4Var("view", view);
		GlobalShaderVars::instance()->setMat4Var("pinnedView", glm::mat4(glm::mat3(view)));
		GlobalShaderVars::instance()->setMat4Var("lightSpaceMatrix", dirShadow.getLightSpaceMatrix());
		GlobalShaderVars::instance()->setTextureVar("shadowMap", 16, GL_TEXTURE_2D, dirShadow.getTextureID());
		GlobalShaderVars::instance()->setTextureVar("pointShadowMap", 17, GL_TEXTURE_CUBE_MAP, pointShadow.getTextureID());
		GlobalShaderVars::instance()->setFloatVar("pointLightFarPlane", pointShadow.getFarPLane());
		GlobalShaderVars::instance()->setFloatVar("time", (GLfloat)currentTime);
		GlobalShaderVars::instance()->setBoolVar("pointLightShadows", rendering::pointLightShadows);
		GlobalShaderVars::instance()->setVec3Var("viewerPos", theCamera->getPosition());

		GlobalShaderVars::instance()->updateAllVars();

		//Render the scene to the frame buffer texture (normal rendering with lighting), and then the texture to the full screen quad
		if (!rendering::deferredMode)	//straightforward rendering, isn't a pain in the ass to implement, and allows for anti-aliasing
		{
			blendingOn();

			msFBO.activateBuffer();

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
			msFBO.renderColorBufferToQuad(screenShaders[rendering::screenShaderId]->getProgramId(), gaussBlurShdr->getProgramId(), renderingQuad, renderingQuad);
		}		
		else	//---G-BUFFER STUFF, deferred rendering, no anti-aliosing for you, slut
		{
			blendingOff();

			//activate gbuffer, fill it's textures with position, normals, color and lightspace data
			gBuff.activateBuffer();

			renderCalls(thegBuffShader->getProgramId(), thegBuffBatchShader->getProgramId(), outlineShader->getProgramId(), outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(),
				models, modelContexts, primitives, lights);

			//render gbuffuer data to fullscreen quad, perform lighting/shadow calculations per-pixel, basically, 
			//instead of for all the fragments for all the vertices in the screen
			lights.setLightingParameters(theDeferredtingShader->getProgramId());

			gBuff.renderColorBufferToQuad(theDeferredtingShader->getProgramId(), 0, renderingQuad, renderingQuad);

			//straightforward-render some things (skybox, in this case and text), will work because gBufer sets the depth of a default FBO properly,
			//and the 3d objects can be drawn over of the screen quad as if they were drawn in a "real" 3d scene
			//just set, don't clear of course, because there is a scene, recreated from the G-buffer texture on top of the full-screen quad now
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			blendingOn();
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

			msFBO.activateBuffer();

			renderCalls(mainShader, mainBatchShader, outlineShader->getProgramId(), outlineBatchShader->getProgramId(), lightSourceShader->getProgramId(),
				models, modelContexts, primitives, lights);

			skyBox.drawCall(skyBoxShader->getProgramId());

			msFBO.renderColorBufferToQuad(screenShaders[rendering::screenShaderId]->getProgramId(), gaussBlurShdr->getProgramId(), renderingQuad, renderingMiniQuad);

			//Reverse rear-view calculations (rear-view)
			theCamera->setPitch(theCamera->getPitch() - 180.0f);
			theCamera->setYaw(theCamera->getYaw() - 360.0f);
		}

		// Swap the buffers
		glfwSwapBuffers(window);

		Inputs::instance()->step(currentTime, deltaTime);
	}

	AsyncModelLoader::instance()->join();

	//As soon as we exit the game loop we would like to properly clean/delete all resources that were allocated
	glfwTerminate();

	return EXIT_SUCCESS;
}