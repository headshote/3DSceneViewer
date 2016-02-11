#pragma once

#define GLEW_STATIC

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <SOIL/SOIL.h>
#include <assimp/Importer.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DataStructures.hpp"

#include "Shader.h"
#include "Camera.h"
#include "Inputs.h"

#include "Renderable.h"
#include "Mesh.h"
#include "RawPrimitive.h"
#include "TextField.h"
#include "SkyBox.h"
#include "Model.h"

#include "FontFactory.h"
#include "SVFont.h"

#include "GlobalShaderVars.h"

#include "LightingSystem.h"

#include "ModelRenderingContext.h"
#include "SingleCallContext.h"
#include "BatchRenderContext.h"

#include "DirectionalShadowMap.h"
#include "PointShadowMap.h"

#include "MultisampledBlurFB.h"
#include "GBuffer.h"

#include "AsyncModelLoader.h"

namespace engine
{
	//Initialisation methods
	GLFWwindow* setUpWindow(int width, int height, GLFWwindow* threadWin);
	void configureOpenGL();
	void printInfo();
	void createContexts();
	void createContexts(std::vector<models::Model>* mdels, std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>>* mdelContexts);
	void loadModels(std::vector<models::Model>* mdels, std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>>* mdelContexts, std::vector<models::AsyncData>* modelQueue, GLFWwindow* window);

	//The engine class
	class REngine
	{
	public:
		REngine();
		~REngine();

		GLboolean renderingLoop();

		void addContextsForModel(const std::string& modelId, std::vector<std::shared_ptr<models::ModelRenderingContext>>& contexts);
		void addModel(models::Model& model);

	private:
		///Everyframe methods
		void blendingOn();
		void blendingOff();
		void clearScreen();
		void renderCalls(const GLuint shaderProgram, const GLuint batchShaderProgram, const GLuint outlineShader, const GLuint outlineBatchShader, const GLuint lightSourceShader);
		void scriptedMovements();
		void checkLoadedModels();

		//viewPort dimesnsions
		GLuint screenWidth = 1280;
		GLuint screenHeight = 720;

		GLuint shadowViewWidth = 1536;
		GLuint shadowViewHeight = 1536;

		//AA
		GLint nFragmentSamples = 4;

		//Window handler
		GLFWwindow* window;

		//Model data
		std::map<std::string, std::vector<std::shared_ptr<models::ModelRenderingContext>>> modelContexts;
		std::vector<models::Model> models;
		std::vector<models::AsyncData> modelQueue;

		std::shared_ptr<renderables::SkyBox> skyBox;

		//Scene shaders
		std::shared_ptr<Shader> theBlinnShader;
		std::shared_ptr<Shader> theBlinnExplodeShader;
		std::shared_ptr<Shader> lightSourceShader;
		std::shared_ptr<Shader> outlineShader;
		//post-processing shaders
		std::shared_ptr<Shader> screenShader;
		std::shared_ptr<Shader> screenShaderBlur;
		std::shared_ptr<Shader> screenShaderEdge;
		std::shared_ptr<Shader> screenShaderGreyScale;
		std::shared_ptr<Shader> screenShaderInversion;
		std::shared_ptr<Shader> screenShaderSharpen;
		std::shared_ptr<Shader> screenShaderHalves;
		std::shared_ptr<Shader> screenShaderNoBloom;
		std::vector< std::shared_ptr<Shader> > screenShaders;
		//sky box
		std::shared_ptr<Shader> skyBoxShader;
		//debug rendering of the mesh normals
		std::shared_ptr<Shader> theNormalsShader;
		//batched rendering
		std::shared_ptr<Shader> theBlinnBatchShader;
		std::shared_ptr<Shader> theBlinnBatchExplodeShader;
		std::shared_ptr<Shader> outlineBatchShader;
		std::shared_ptr<Shader> theBatchNormalsShader;
		//for directional shadow mapping
		std::shared_ptr<Shader> shadowMapShader;
		std::shared_ptr<Shader> shadowMapBatchShader;
		//for point shadow mapping
		std::shared_ptr<Shader> pointShadowMapShader;
		std::shared_ptr<Shader> pointShadowMapBatchShader;
		//gauss blur
		std::shared_ptr<Shader> gaussBlurShdr;
		//deferred rendering g-buffer shaders
		std::shared_ptr<Shader> thegBuffShader;
		std::shared_ptr<Shader> thegBuffBatchShader;
		std::shared_ptr<Shader> theDeferredtingShader;
		//text rendering shader
		std::shared_ptr<Shader> textShader;

		//selection of shaders
		GLuint screenShaderId;

		GLfloat hdrExposure;

		GLboolean renderNormals;

		GLboolean explodeMode;

		GLboolean rearView;

		GLboolean highlightModels;

		GLboolean dotMode;

		//whether the first point light generates shadows
		GLboolean pointLightShadows;

		//camera
		std::shared_ptr<Camera> theCamera;

		//Frame buffer to render to
		std::shared_ptr<framebuffers::MultisampledBlurFB> msFBO;

		//G-buffer, for deferred rendering
		std::shared_ptr<framebuffers::GBuffer> gBuff;
		GLboolean deferredMode;

		//because of post-processing
		std::shared_ptr<renderables::RawPrimitive> rq;
		std::shared_ptr<renderables::RawPrimitive> rmq;
		GLuint renderingQuad;
		GLuint renderingMiniQuad;

		//shadows
		std::shared_ptr<shadows::DirectionalShadowMap> dirShadow;
		std::shared_ptr<shadows::PointShadowMap> pointShadow;

		std::shared_ptr<renderables::TextField> textField1;

		//Lightsources and their debug rendering (wireframed)
		std::shared_ptr<lighting::LightingSystem> lights;
		std::vector<models::Model> primitives;

		//for time intervals
		GLdouble lastFrame;
	};

}
