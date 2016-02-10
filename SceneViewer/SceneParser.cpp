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

std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>> parseFile(const std::string& filename)
{
	std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>> modelContexts;



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

	return modelContexts;
}