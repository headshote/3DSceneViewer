#include <stdlib.h>

#include "REngine.h"

void checkLoadedModels(std::vector<models::AsyncData>& modelQueue, engine::REngine& theEngine)
{
	while (modelQueue.size() > 0)
	{
		models::AsyncData& vertexData = modelQueue[modelQueue.size() - 1];
		models::Model mdl1(vertexData);
		modelQueue.pop_back();

		theEngine.addModel(mdl1);
	}
}

int main()
{
	engine::REngine theEngine;

	std::vector<models::AsyncData> modelQueue;

	while (theEngine.renderingLoop())
	{
		checkLoadedModels(modelQueue, theEngine);
	}

	return EXIT_SUCCESS;
}