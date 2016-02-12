#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>

#include "ModelRenderingContext.h"
#include "utils.h"

namespace scenes
{
	class SceneParser
	{
	public:
		static std::shared_ptr<SceneParser> getInstance();

		~SceneParser();

		void parseFile(const std::string& filename);

		std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>> getContexts();
		std::vector<models::Model> getModels();

	private:
		static std::shared_ptr<SceneParser> instance;

		SceneParser();

		bool endReached(const std::string& fileContents);
		std::string getLine(const std::string& fileContents);

		unsigned long fileCursor;

		std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>> sceneContexts;
		std::vector<models::Model> sceneModels;

	};
}