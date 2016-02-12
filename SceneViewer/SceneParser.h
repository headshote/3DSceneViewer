#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>

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
		std::vector<models::Model> getModels();

		void setEngine(engine::REngine* eng);

	private:
		static std::shared_ptr<SceneParser> instance;

		explicit SceneParser();

		bool endReached(const std::string& fileContents);
		std::string getLine(const std::string& fileContents);
		void parseLine(const std::string& fileLine);

		std::string currentModelId;

		unsigned long fileCursor;

		models::AsyncModelLoader asyncLoader;

		engine::REngine* theEngine;
		std::vector<models::AsyncData>* modelQueue;

		std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>> sceneContexts;
		std::vector<models::Model> sceneModels;

	};
}