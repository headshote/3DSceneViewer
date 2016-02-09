#include <stdlib.h>

#include "REngine.h"


int main()
{
	engine::REngine theEngine;

	theEngine.renderingLoop();

	return EXIT_SUCCESS;
}