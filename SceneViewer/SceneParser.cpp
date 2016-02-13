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
	lineCursor = 0;

	std::string parsedContent;

	while (lineCursor < fileLine.length())
	{
		char curChr = fileLine.at(lineCursor);
		parsedContent += curChr;

		if (curChr == ' '|| curChr == '\t')	//word-by-word analysys, refresh parsing after meeting a space
		{
			parsedContent = "";
		}

		if (parsedContent == "model")
		{
			getQuotes(fileLine);
			currentModelId = fileLine.substr(firstQuoteId + 1, secondQuoteId - firstQuoteId-1);

			sceneContexts[currentModelId] = std::vector<std::shared_ptr<ModelRenderingContext>>();
			return;
		}
		else if (parsedContent == "context")
		{
			getQuotes(fileLine);
			std::string conTextType = fileLine.substr(firstQuoteId + 1, secondQuoteId - firstQuoteId - 1);

			//Create a new context, and set the appropriate state
			if (conTextType == "single")
				sceneContexts[currentModelId].push_back(std::shared_ptr<models::ModelRenderingContext>(
					new models::SingleCallContext()));
			else if (conTextType == "batch")
				sceneContexts[currentModelId].push_back(std::shared_ptr<models::ModelRenderingContext>(
					new models::BatchRenderContext()));

			cutTransformId = 0;

			return;
		}
		else if (parsedContent == "translation")
		{	
			getQuotes(fileLine);
			std::string content = fileLine.substr(firstQuoteId + 1, secondQuoteId - firstQuoteId - 1);

			std::vector<float> contentNums = parseVector(content);

			curTranslation.x = contentNums[0];
			curTranslation.y = contentNums[1];
			curTranslation.z = contentNums[2];

			//andvance, since we've scanned enough of the string
			lineCursor = secondQuoteId;
		}
		else if (parsedContent == "rotationAxis")
		{
			getQuotes(fileLine);
			std::string content = fileLine.substr(firstQuoteId + 1, secondQuoteId - firstQuoteId - 1);

			std::vector<float> contentNums = parseVector(content);

			curRotAxis.x = contentNums[0];
			curRotAxis.y = contentNums[1];
			curRotAxis.z = contentNums[2];

			//andvance, since we've scanned enough of the string
			lineCursor = secondQuoteId;
		}
		else if (parsedContent == "rotationAngle")
		{	
			getQuotes(fileLine);
			std::string content = fileLine.substr(firstQuoteId + 1, secondQuoteId - firstQuoteId - 1);

			curRotAngle = atof(content.c_str());

			//andvance, since we've scanned enough of the string
			lineCursor = secondQuoteId;
		}
		else if (parsedContent == "scale")
		{	
			getQuotes(fileLine);
			std::string content = fileLine.substr(firstQuoteId + 1, secondQuoteId - firstQuoteId - 1);

			std::vector<float> contentNums = parseVector(content);

			curScale.x = contentNums[0];
			curScale.y = contentNums[1];
			curScale.z = contentNums[2];

			//since we've met scale, that means it's the end of the transformation, flush everything, exit loop
			std::vector<std::shared_ptr<ModelRenderingContext>> curContexts = sceneContexts[currentModelId];
			curContexts[curContexts.size() - 1]->appendTranslation(curTranslation);
			curContexts[curContexts.size() - 1]->appendRotation(curRotAxis, curRotAngle);
			curContexts[curContexts.size() - 1]->appendScale(curScale);

			++cutTransformId;			
			return;
		}

		++lineCursor;
	}
}

std::vector<float> SceneParser::parseVector(const std::string& content)
{
	std::vector<float> contentNums;
	int contentCursor = 0;
	int commaId = 0;
	while ((commaId = content.find_first_of(',', contentCursor)) > -1)
	{
		std::string floatStr = content.substr(contentCursor, commaId - contentCursor);
		contentNums.push_back(atof(floatStr.c_str()));
		contentCursor = commaId + 1;
	}
	contentNums.push_back(atof(content.substr(contentCursor, content.length() - contentCursor).c_str()));

	return contentNums;
}

void SceneParser::getQuotes(const std::string& fileLine)
{
	firstQuoteId = fileLine.find_first_of('\"', lineCursor);
	secondQuoteId = fileLine.find_first_of('\"', firstQuoteId + 1);
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