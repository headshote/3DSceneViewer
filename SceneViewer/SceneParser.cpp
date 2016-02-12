#include "SceneParser.h"

using namespace scene;
using namespace models;

/*static*/ std::shared_ptr<SceneParser> SceneParser::instance;
/*static*/ std::shared_ptr<SceneParser> SceneParser::getInstance()
{
	if (instance.get() == nullptr)
		instance.reset(new SceneParser());

	return instance;
}

SceneParser::SceneParser()
{
}


SceneParser::~SceneParser()
{
}

void SceneParser::parseFile(const std::string& filename)
{

	sceneContexts["models/nanosuit/nanosuit.obj"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	sceneContexts["grass1"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	sceneContexts["glass1"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	sceneContexts["models/rock/rock.obj"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	sceneContexts["models/planet/planet.obj"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	sceneContexts["floor1"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	sceneContexts["brick1"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	sceneContexts["brick2"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	sceneContexts["brick3"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
	sceneContexts["wood1"] = std::vector<std::shared_ptr<ModelRenderingContext>>();

}