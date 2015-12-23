#ifndef FONT_FACTORY
#define FONT_FACTORY

#include <memory>
#include <GL/glew.h>

#include "SVFont.h"
#include "TextField.h"

namespace textandfonts
{
	class FontFactory
	{
	public:
		~FontFactory();

		static std::shared_ptr<FontFactory> instance();

		std::shared_ptr<SVFont> CreateFont(const std::string& filePath = "fonts/arial.ttf", const GLuint fontSize = 48, const GLuint scrWidth = 1280, const GLuint scrHeight = 720);

		std::shared_ptr<::renderables::TextField> CreateRenderableText(std::shared_ptr<SVFont> fontObject, const std::string& text);
		std::shared_ptr<::renderables::TextField> CreateRenderableText(const std::string& fontPath, const GLuint fontSize, const GLuint scrWidth, const GLuint scrHeight, const std::string& text);

	private:
		//no copy constructors and assignments
		FontFactory(const FontFactory&);
		FontFactory& operator=(const FontFactory&);
		//no public constructors, singleton
		FontFactory();

		static std::shared_ptr<FontFactory> theInstance;

		GLuint quadVAO;
		GLuint quadVBO;

		void generateFontQuad();

	};
}

#endif