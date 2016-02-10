#include "utils.h"

std::string* utils::readFile(const GLchar* filePath)
{
	// 1. Retrieve the vertex/fragment source code from filePath
	std::string* fileContents = nullptr;
	std::ifstream theFile;

	// ensures ifstream objects can throw exceptions:
	theFile.exceptions(std::ifstream::badbit);

	try
	{
		std::stringstream fileStream;

		// Open file
		theFile.open(filePath);
		// Read file's buffer contents into streams
		fileStream << theFile.rdbuf();
		// close file handlers
		theFile.close();
		// Convert stream into GLchar array
		fileContents = new std::string(fileStream.str());
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	return fileContents;
}
