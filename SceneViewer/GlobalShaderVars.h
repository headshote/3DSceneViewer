#pragma once

#include <map>
#include <vector>
#include <memory>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace shadervars
{
	class GlobalShaderVars
	{
	public:
		static std::shared_ptr<GlobalShaderVars> instance();

		void addUIntVar(const std::string& varName);
		void setUIntVar(const std::string& varName, const GLuint value);
		void addIntVar(const std::string& varName);
		void setIntVar(const std::string& varName, const GLint value);
		void addFloatVar(const std::string& varName);
		void setFloatVar(const std::string& varName, const GLfloat value);
		void addBoolVar(const std::string& varName);
		void setBoolVar(const std::string& varName, const GLboolean value);
		void addVec3Var(const std::string& varName);
		void setVec3Var(const std::string& varName, const glm::vec3& value);
		void addMat4Var(const std::string& varName);
		void setMat4Var(const std::string& varName, const glm::mat4& value);
		void addTextureVar(const std::string& varName);
		void setTextureVar(const std::string& varName, const GLuint value, const GLenum textureType, const GLuint textureID);

		void subscribeShaderToVar(const GLuint shader, const std::string& varName);

		void updateAllVars();

	private:
		static std::shared_ptr<GlobalShaderVars> theInstance;

		//no copy constructors and assignments
		GlobalShaderVars(const GlobalShaderVars&);
		GlobalShaderVars& operator=(const GlobalShaderVars&);
		GlobalShaderVars();

		std::map<std::string, std::vector<GLuint>> varShaders;	//list of all the shaders, subscribed to this var

		std::map<std::string, GLuint> uints;
		std::map<std::string, GLint> ints;
		std::map<std::string, GLfloat> floats;
		std::map<std::string, GLboolean> bools;
		std::map<std::string, glm::vec3> vec3s;
		std::map<std::string, glm::mat4> mat4s;
		std::map<std::string, std::pair <GLuint, std::pair <GLuint, GLenum> >> textures;

	};
}
