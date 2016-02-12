#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>

#include "ModelRenderingContext.h"
#include "utils.h"

namespace scene
{
	class SceneParser
	{
	public:
		static std::shared_ptr<SceneParser> getInstance();

		~SceneParser();

		void parseFile(const std::string& filename);

	private:
		static std::shared_ptr<SceneParser> instance;

		SceneParser();

		std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>> sceneContexts;
		std::vector<models::Model> sceneModels;

	};
}