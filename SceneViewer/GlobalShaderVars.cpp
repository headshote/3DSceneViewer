#include "GlobalShaderVars.h"

using namespace shadervars;

GlobalShaderVars::GlobalShaderVars()
{
}

/*static*/ std::shared_ptr<GlobalShaderVars> GlobalShaderVars::theInstance;

/*static*/ std::shared_ptr<GlobalShaderVars> GlobalShaderVars::instance()
{
	if (!theInstance.get())
		theInstance.reset(new GlobalShaderVars());
	return theInstance;
}

void GlobalShaderVars::addUIntVar(const std::string& varName)
{
	uints[varName] = 0;
	varShaders[varName] = std::vector < GLuint >();
}
void GlobalShaderVars::setUIntVar(const std::string& varName, const GLuint value)
{
	uints[varName] = value;
}
void GlobalShaderVars::addIntVar(const std::string& varName)
{
	ints[varName] = 0;
	varShaders[varName] = std::vector < GLuint >();
}
void GlobalShaderVars::setIntVar(const std::string& varName, const GLint value)
{
	ints[varName] = value;
}
void GlobalShaderVars::addFloatVar(const std::string& varName)
{
	floats[varName] = 0.0f;
	varShaders[varName] = std::vector < GLuint >();
}
void GlobalShaderVars::setFloatVar(const std::string& varName, const GLfloat value)
{
	floats[varName] = value;
}
void GlobalShaderVars::addBoolVar(const std::string& varName)
{
	bools[varName] = false;
	varShaders[varName] = std::vector < GLuint >();
}
void GlobalShaderVars::setBoolVar(const std::string& varName, const GLboolean value)
{
	bools[varName] = value;
}
void GlobalShaderVars::addVec3Var(const std::string& varName)
{
	vec3s[varName] = glm::vec3(0.0f);
	varShaders[varName] = std::vector < GLuint >();
}
void GlobalShaderVars::setVec3Var(const std::string& varName, const glm::vec3& value)
{
	vec3s[varName] = value;
}
void GlobalShaderVars::addMat4Var(const std::string& varName)
{
	mat4s[varName] = glm::mat4(0.0f);
	varShaders[varName] = std::vector < GLuint >();
}
void GlobalShaderVars::setMat4Var(const std::string& varName, const glm::mat4& value)
{
	mat4s[varName] = value;
}
void GlobalShaderVars::addTextureVar(const std::string& varName)
{
	textures[varName] = std::pair <GLuint, std::pair <GLuint, GLenum> >(0, std::pair <GLuint, GLenum>(0, GL_TEXTURE_2D));
	varShaders[varName] = std::vector < GLuint >();
}
void GlobalShaderVars::setTextureVar(const std::string& varName, const GLuint value, const GLenum textureType, const GLuint textureID)
{
	textures[varName] = std::pair <GLuint, std::pair <GLuint, GLenum> >(value, std::pair <GLuint, GLenum>(textureID, textureType));
}

void GlobalShaderVars::subscribeShaderToVar(const GLuint shader, const std::string& varName)
{
	varShaders[varName].push_back(shader);
}

void GlobalShaderVars::updateAllVars()
{
	for (auto iterator = uints.cbegin(); iterator != uints.cend(); ++iterator)
	{
		std::pair<std::string, GLuint> varNameAndValue = *iterator;
		const std::string& varName = varNameAndValue.first;
		const GLuint varValue = varNameAndValue.second;

		for (auto shadersIterator = varShaders[varName].cbegin(); shadersIterator != varShaders[varName].cend(); ++shadersIterator)
		{
			const GLuint shaderId = *shadersIterator;

			glUseProgram(shaderId);
			glUniform1ui(glGetUniformLocation(shaderId, varName.c_str()), varValue);
		}
	}

	for (auto iterator = ints.cbegin(); iterator != ints.cend(); ++iterator)
	{
		std::pair<std::string, GLint> varNameAndValue = *iterator;
		const std::string& varName = varNameAndValue.first;
		const GLint varValue = varNameAndValue.second;

		for (auto shadersIterator = varShaders[varName].cbegin(); shadersIterator != varShaders[varName].cend(); ++shadersIterator)
		{
			const GLuint shaderId = *shadersIterator;

			glUseProgram(shaderId);
			glUniform1i(glGetUniformLocation(shaderId, varName.c_str()), varValue);
		}
	}

	for (auto iterator = floats.cbegin(); iterator != floats.cend(); ++iterator)
	{
		std::pair<std::string, GLfloat> varNameAndValue = *iterator;
		const std::string& varName = varNameAndValue.first;
		const GLfloat varValue = varNameAndValue.second;

		for (auto shadersIterator = varShaders[varName].cbegin(); shadersIterator != varShaders[varName].cend(); ++shadersIterator)
		{
			const GLuint shaderId = *shadersIterator;

			glUseProgram(shaderId);
			glUniform1f(glGetUniformLocation(shaderId, varName.c_str()), varValue);
		}
	}

	for (auto iterator = bools.cbegin(); iterator != bools.cend(); ++iterator)
	{
		std::pair<std::string, GLboolean> varNameAndValue = *iterator;
		const std::string& varName = varNameAndValue.first;
		const GLboolean varValue = varNameAndValue.second;

		for (auto shadersIterator = varShaders[varName].cbegin(); shadersIterator != varShaders[varName].cend(); ++shadersIterator)
		{
			const GLuint shaderId = *shadersIterator;

			glUseProgram(shaderId);
			glUniform1i(glGetUniformLocation(shaderId, varName.c_str()), varValue);
		}
	}

	for (auto iterator = vec3s.cbegin(); iterator != vec3s.cend(); ++iterator)
	{
		std::pair<std::string, glm::vec3> varNameAndValue = *iterator;
		const std::string& varName = varNameAndValue.first;
		const glm::vec3& varValue = varNameAndValue.second;

		for (auto shadersIterator = varShaders[varName].cbegin(); shadersIterator != varShaders[varName].cend(); ++shadersIterator)
		{
			const GLuint shaderId = *shadersIterator;

			glUseProgram(shaderId);
			glUniform3f(glGetUniformLocation(shaderId, varName.c_str()), varValue.x, varValue.y, varValue.z);
		}
	}

	for (auto iterator = mat4s.cbegin(); iterator != mat4s.cend(); ++iterator)
	{
		std::pair<std::string, glm::mat4> varNameAndValue = *iterator;
		const std::string& varName = varNameAndValue.first;
		const glm::mat4& varValue = varNameAndValue.second;

		for (auto shadersIterator = varShaders[varName].cbegin(); shadersIterator != varShaders[varName].cend(); ++shadersIterator)
		{
			const GLuint shaderId = *shadersIterator;

			glUseProgram(shaderId);
			glUniformMatrix4fv(glGetUniformLocation(shaderId, varName.c_str()), 1,
				GL_FALSE,	//transpose?
				glm::value_ptr(varValue));
		}
	}

	for (auto iterator = textures.cbegin(); iterator != textures.cend(); ++iterator)
	{
		std::pair<std::string, std::pair <GLuint, std::pair <GLuint, GLenum> >> varNameAndValue = *iterator;
		const std::string& varName = varNameAndValue.first;
		const GLint varValue = varNameAndValue.second.first;
		const GLuint textureId = varNameAndValue.second.second.first;
		const GLenum textureType = varNameAndValue.second.second.second;

		for (auto shadersIterator = varShaders[varName].cbegin(); shadersIterator != varShaders[varName].cend(); ++shadersIterator)
		{
			const GLuint shaderId = *shadersIterator;

			glUseProgram(shaderId);
			glActiveTexture(GL_TEXTURE0 + varValue);
			glBindTexture(textureType, textureId);
			glUniform1i(glGetUniformLocation(shaderId, varName.c_str()), varValue);
		}
	}
}