#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>

#include "ModelRenderingContext.h"

namespace scene
{
	class SceneParser
	{
	public:
		static std::shared_ptr<SceneParser> getInstance();

		SceneParser();
		~SceneParser();

		std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>> parseFile(const std::string& filename);

	private:
		static std::shared_ptr<SceneParser> instance;

	};
}