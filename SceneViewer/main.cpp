#include <stdlib.h>

#include "REngine.h"
#include "SceneParser.h"

static GLfloat transparentVertices[] = {
	// Positions		//Normals					// Texture Coords (swapped y coordinates because texture is flipped upside down)
	0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f
};

static GLfloat planeVertices[] = {
	// Positions			 // Normals         // Texture Coords
	60.0f, -0.5f, 60.0f, 0.0f, 1.0f, 0.0f, 50.0f, 0.0f,
	-60.0f, -0.5f, 60.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	-60.0f, -0.5f, -60.0f, 0.0f, 1.0f, 0.0f, 0.0f, 50.0f,

	60.0f, -0.5f, 60.0f, 0.0f, 1.0f, 0.0f, 50.0f, 0.0f,
	-60.0f, -0.5f, -60.0f, 0.0f, 1.0f, 0.0f, 0.0f, 50.0f,
	60.0f, -0.5f, -60.0f, 0.0f, 1.0f, 0.0f, 50.0f, 50.0f
};

static GLfloat wallVertices[] = {
	// Positions		// Normals			// Texture Coords 
	0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
	1.0f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f
};

/**
*	Later on, contexts should be created from a scene file, for now, the whole scene is hardcoded like this
*/
void createContexts(std::vector<models::Model>& models, engine::REngine& theEngine)
{
	std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>> modelContexts;

	modelContexts["models/nanosuit/nanosuit.obj"] = std::vector<std::shared_ptr<models::ModelRenderingContext>>();
	modelContexts["grass1"] = std::vector<std::shared_ptr<models::ModelRenderingContext>>();
	modelContexts["glass1"] = std::vector<std::shared_ptr<models::ModelRenderingContext>>();
	modelContexts["models/rock/rock.obj"] = std::vector<std::shared_ptr<models::ModelRenderingContext>>();
	modelContexts["models/planet/planet.obj"] = std::vector<std::shared_ptr<models::ModelRenderingContext>>();
	modelContexts["floor1"] = std::vector<std::shared_ptr<models::ModelRenderingContext>>();
	modelContexts["brick1"] = std::vector<std::shared_ptr<models::ModelRenderingContext>>();
	modelContexts["brick2"] = std::vector<std::shared_ptr<models::ModelRenderingContext>>();
	modelContexts["brick3"] = std::vector<std::shared_ptr<models::ModelRenderingContext>>();
	modelContexts["wood1"] = std::vector<std::shared_ptr<models::ModelRenderingContext>>();

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
	modelContexts["models/nanosuit/nanosuit.obj"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::SingleCallContext(glm::vec3(1.0f, 0.0f, -2.0f), glm::vec3(0.33f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f)));
	modelContexts["models/nanosuit/nanosuit.obj"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::BatchRenderContext(translations, scales, rotationAxes, rotationAngles)));

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
	modelContexts["models/rock/rock.obj"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::BatchRenderContext(translations, scales, rotationAxes, rotationAngles)));

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
	modelContexts["models/planet/planet.obj"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::BatchRenderContext(translations, scales, rotationAxes, rotationAngles)));

	modelContexts["grass1"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::SingleCallContext(glm::vec3(-1.5f, 0.0f, -0.48f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["glass1"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::SingleCallContext(glm::vec3(0.5f, 0.0f, -0.48f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["floor1"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::SingleCallContext(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["brick1"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::SingleCallContext(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["brick1"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::SingleCallContext(glm::vec3(0.0f, 0.0f, 0.52f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false)));
	modelContexts["brick2"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::SingleCallContext(glm::vec3(1.1f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["brick2"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::SingleCallContext(glm::vec3(1.1f, 0.0f, 0.52f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false)));
	modelContexts["brick3"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::SingleCallContext(glm::vec3(-1.1f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["brick3"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::SingleCallContext(glm::vec3(-1.1f, 0.0f, 0.52f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false)));
	modelContexts["wood1"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::SingleCallContext(glm::vec3(-2.1f, 0.5f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, false)));
	modelContexts["wood1"].push_back(std::shared_ptr<models::ModelRenderingContext>(new models::SingleCallContext(glm::vec3(-2.1f, 0.0f, 0.52f), glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), -90.0f, false)));

	//Shove all this into the negine
	for (GLuint i = 0; i < models.size(); ++i)
	{
		models::Model& model = models[i];
		theEngine.addModel(model);
	}

	for (auto iterator = modelContexts.begin(); iterator != modelContexts.end(); iterator++) {
		theEngine.addContextsForModel(iterator->first, iterator->second);
		// iterator->first = key
		// iterator->second = value
	}
}

/**
* List of the models to load at the beginning of the program should be in the scene file
*/
void loadModels(std::vector<models::AsyncData>* modelQueue, engine::REngine& theEngine, models::AsyncModelLoader& asyncLoader)
{
	std::vector<models::Model> models;

	//0
	std::string nanoPath = "models/nanosuit/nanosuit.obj";
	asyncLoader.loadModel(&nanoPath, modelQueue);

	//1
	GLuint grassTexture[] = { models::loadTexture("textures/grass.png", true, false), models::loadTexture("textures/mt_specular.png") };
	models.push_back(models::Model(new renderables::Mesh(transparentVertices, sizeof(transparentVertices), grassTexture, sizeof(grassTexture) / sizeof(GLuint)), "grass1"));

	//2
	GLuint glassTexture[] = { models::loadTexture("textures/blending_transparent_window.png", false, false), models::loadTexture("textures/mt_specular.png") };
	models.push_back(models::Model(new renderables::Mesh(transparentVertices, sizeof(transparentVertices), glassTexture, sizeof(glassTexture) / sizeof(GLuint), 0), "glass1"));

	//3
	std::string rockPath = "models/rock/rock.obj";
	asyncLoader.loadModel(&rockPath, modelQueue);

	////4
	std::string planetPath = "models/planet/planet.obj";
	asyncLoader.loadModel(&planetPath, modelQueue);

	//5
	GLuint metalTexture[] = { models::loadTexture("textures/metal1.jpg", false, false), models::loadTexture("textures/metal1_specular.png") };
	models.push_back(models::Model(new renderables::Mesh(planeVertices, sizeof(planeVertices), metalTexture, sizeof(metalTexture) / sizeof(GLuint)), "floor1"));

	//6
	GLuint brickTexture[] = { models::loadTexture("textures/brickwall.jpg", false, false), models::loadTexture("textures/mt_specular.png"), models::loadTexture("textures/brickwall_normal.jpg") };
	models.push_back(models::Model(new renderables::Mesh(wallVertices, sizeof(wallVertices), brickTexture, sizeof(brickTexture) / sizeof(GLuint)), "brick1"));

	//7
	GLuint brickPlainTexture[] = { models::loadTexture("textures/brickwall.jpg", false, false), models::loadTexture("textures/mt_specular.png") };
	models.push_back(models::Model(new renderables::Mesh(wallVertices, sizeof(wallVertices), brickPlainTexture, sizeof(brickPlainTexture) / sizeof(GLuint)), "brick2"));

	//8
	GLuint brickParallaxTexture[] = {
		models::loadTexture("textures/parallax_brix/bricks2.jpg", false, false),
		models::loadTexture("textures/parallax_brix/specular.png"),
		models::loadTexture("textures/parallax_brix/bricks2_normal.jpg"),
		models::loadTexture("textures/parallax_brix/bricks2_disp.jpg") };
	models.push_back(models::Model(new renderables::Mesh(wallVertices, sizeof(wallVertices), brickParallaxTexture, sizeof(brickParallaxTexture) / sizeof(GLuint)), "brick3"));

	//9
	GLuint tbParallaxTexture[] = {
		models::loadTexture("textures/woodbox/wood.png", false, false),
		models::loadTexture("textures/woodbox/specular.png"),
		models::loadTexture("textures/woodbox/toy_box_normal.png"),
		models::loadTexture("textures/woodbox/toy_box_disp.png") };
	models.push_back(models::Model(new renderables::Mesh(wallVertices, sizeof(wallVertices), tbParallaxTexture, sizeof(tbParallaxTexture) / sizeof(GLuint)), "wood1"));

	createContexts(models, theEngine);

	glFlush();
}

void checkLoadedModels(std::vector<models::AsyncData>& modelQueue, engine::REngine& theEngine)
{
	while (modelQueue.size() > 0)
	{
		GLuint lastId = modelQueue.size() - 1;
		models::AsyncData* vertexData = &modelQueue[lastId];
		models::Model mdl1(*vertexData);
		modelQueue.pop_back();

		theEngine.addModel(mdl1);
	}
}

int main()
{
	engine::REngine theEngine;

	models::AsyncModelLoader asyncLoader;
	std::vector<models::AsyncData> modelQueue;

	loadModels(&modelQueue, theEngine, asyncLoader);

	scenes::SceneParser::getInstance()->setEngine(&theEngine);
	scenes::SceneParser::getInstance()->parseFile("scenes/default.scn", &modelQueue);

	while (theEngine.renderingLoop())
	{
		if (asyncLoader.isDone())
			checkLoadedModels(modelQueue, theEngine);
	}

	return EXIT_SUCCESS;
}