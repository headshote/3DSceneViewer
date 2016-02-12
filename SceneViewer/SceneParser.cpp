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

void SceneParser::parseFile(const std::string& filename, std::vector<models::AsyncData>* mQueue)
{
	modelQueue = mQueue;

	std::string fileContents = utils::readFile(filename.c_str());
	fileCursor = 0;

	while (!endReached(fileContents))
	{
		std::string fileLine = getLine(fileContents);
		parseLine(fileLine);
		//std::cout << fileLine << "\n";
	}
	return;
}

void SceneParser::parseLine(const std::string& fileLine)
{
	unsigned int lineCursor = 0;

	std::string parsedContent;

	while (lineCursor < fileLine.length())
	{
		parsedContent += fileLine.at(lineCursor);

		if (parsedContent == " " || parsedContent == "\t")	//word-by-word analysys, refresh parsing after meeting a space
		{
			parsedContent = "";
		}
		else if (parsedContent == "model")
		{
			int firstQuoteId = fileLine.find_first_of('\"', lineCursor);
			int secondQuoteId = fileLine.find_first_of('\"', firstQuoteId+1);
			currentModelId = fileLine.substr(firstQuoteId + 1, secondQuoteId - firstQuoteId-1);

			sceneContexts[currentModelId] = std::vector<std::shared_ptr<ModelRenderingContext>>();
			return;
		}
		else if (parsedContent == "context")
		{
			int firstQuoteId = fileLine.find_first_of('\"', lineCursor);
			int secondQuoteId = fileLine.find_first_of('\"', firstQuoteId + 1);
			std::string conTextType = fileLine.substr(firstQuoteId + 1, secondQuoteId - firstQuoteId - 1);

			if (conTextType == "single")
				sceneContexts[currentModelId].push_back(std::shared_ptr<models::ModelRenderingContext>(
					new models::SingleCallContext()));
			else if (conTextType == "batch")
				sceneContexts[currentModelId].push_back(std::shared_ptr<models::ModelRenderingContext>(
					new models::BatchRenderContext()));
			return;
		}

		++lineCursor;
	}
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

void SceneParser::setEngine(engine::REngine* eng)
{
	theEngine = eng;
}

std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>> SceneParser::getContexts()
{
	return sceneContexts;
}

std::vector<models::Model> SceneParser::getModels()
{
	return sceneModels;
}