#include "SceneParser.h"

using namespace scenes;
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
	std::string fileContents = utils::readFile(filename.c_str());
	fileCursor = 0;

	while (!endReached(fileContents))
	{
		std::string fileLine = getLine(fileContents);
		std::cout << fileLine << "\n";
	}

	//sceneContexts["models/nanosuit/nanosuit.obj"] = std::vector<std::shared_ptr<ModelRenderingContext>>();
}

bool SceneParser::endReached(const std::string& fileContents)
{
	return fileCursor >= fileContents.size();
}

std::string SceneParser::getLine(const std::string& fileContents)
{
	int newLneid = fileContents.find_first_of('\n', fileCursor);
	std::string fileLine = fileContents.substr(fileCursor, newLneid - fileCursor);
	if (newLneid > -1)
		fileCursor = newLneid + 1;
	else
		fileCursor = fileContents.size();
	return fileLine;
}


std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>> SceneParser::getContexts()
{
	return sceneContexts;
}

std::vector<models::Model> SceneParser::getModels()
{
	return sceneModels;
}