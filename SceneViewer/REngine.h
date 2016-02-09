#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "rendering.h"
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

using namespace renderables;
using namespace textandfonts;
using namespace shadervars;
using namespace lighting;
using namespace models;
using namespace shadows;
using namespace framebuffers;

namespace engine
{
	//Initialisation methods
	GLFWwindow* setUpWindow(int width, int height, GLFWwindow* threadWin);
	void configureOpenGL();
	void printInfo();
	void createContexts();
	void createContexts(std::vector<Model>* mdels, std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>>* mdelContexts);
	void loadModels(std::vector<Model>* mdels, std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>>* mdelContexts, std::vector<AsyncData>* modelQueue, GLFWwindow* window);

	//The engine class
	class REngine
	{
	public:
		REngine();
		~REngine();

		void renderingLoop();

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
		std::map<std::string, std::vector<std::shared_ptr<ModelRenderingContext>>> modelContexts;
		std::vector<Model> models;
		std::vector<AsyncData> modelQueue;

		std::shared_ptr<SkyBox> skyBox;

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

		//camera
		std::shared_ptr<Camera> theCamera;

		//Frame buffer to render to
		std::shared_ptr<MultisampledBlurFB >msFBO;

		//G-buffer, for deferred rendering
		std::shared_ptr<GBuffer> gBuff;

		//because of post-processing
		GLuint renderingQuad;
		GLuint renderingMiniQuad;

		//shadows
		std::shared_ptr<DirectionalShadowMap> dirShadow;
		std::shared_ptr<PointShadowMap> pointShadow;

		std::shared_ptr<TextField> textField1;

		//Lightsources and their debug rendering (wireframed)
		std::shared_ptr<LightingSystem> lights;
		std::vector<Model> primitives;

		//for time intervals
		GLdouble lastFrame;
	};

}
