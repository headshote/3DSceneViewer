#pragma once

#include <stdlib.h>

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "ModelRenderingContext.h"
#include "SingleCallContext.h"
#include "BatchRenderContext.h"
#include "REngine.h"
#include "utils.h"

namespace scenes
{
	class SceneParser
	{
	public:
		static std::shared_ptr<SceneParser> getInstance();

		~SceneParser();

		void parseFile(const std::string& filename, std::vector<models::AsyncData>* mQueue);

		std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>> getContexts();

		void setEngine(engine::REngine* eng);

		bool isLoaderDone();

	private:
		static std::shared_ptr<SceneParser> instance;

		explicit SceneParser();

		bool endReached(const std::string& fileContents);
		std::string getLine(const std::string& fileContents);
		void parseLine(const std::string& fileLine);

		void getQuotes(const std::string& fileLine);

		std::vector<float> parseVector(const std::string& content);

		//parsing state vars
		unsigned long fileCursor;

		unsigned int lineCursor;

		int firstQuoteId;
		int secondQuoteId;

		glm::vec3 curTranslation;
		glm::vec3 curRotAxis;
		glm::vec3 curScale;
		float curRotAngle;
		int cutTransformId;

		std::string currentModelId;

		//model loading module
		models::AsyncModelLoader asyncLoader;

		//engine/client interaction vars
		engine::REngine* theEngine;
		std::vector<models::AsyncData>* modelQueue;

		//the goodies data storage
		std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>> sceneContexts;

	};
}