#include "FontFactory.h"

using namespace textandfonts;

/*static*/ std::shared_ptr<FontFactory> FontFactory::theInstance;

FontFactory::FontFactory()
{
	generateFontQuad();
}


FontFactory::~FontFactory()
{
	glDeleteBuffers(1, &quadVBO);
	glDeleteVertexArrays(1, &quadVAO);
}

void FontFactory::generateFontQuad()
{
	GLuint fntVAO, fntVBO;
	glGenVertexArrays(1, &fntVAO);
	glGenBuffers(1, &fntVBO);
	glBindVertexArray(fntVAO);
	glBindBuffer(GL_ARRAY_BUFFER, fntVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	quadVAO = fntVAO;
	quadVBO = fntVBO;
}

std::shared_ptr<SVFont> FontFactory::CreateFont(const std::string& filePath, const GLuint fontSize, const GLuint scrWidth, const GLuint scrHeight)
{
	return std::shared_ptr<SVFont> (new SVFont(quadVAO, quadVBO, filePath, fontSize, scrWidth, scrHeight));
}

std::shared_ptr<::renderables::TextField> FontFactory::CreateRenderableText(std::shared_ptr<SVFont> font, const std::string& text)
{
	return std::shared_ptr<renderables::TextField>(new renderables::TextField(font, text));
}

std::shared_ptr<::renderables::TextField> FontFactory::CreateRenderableText(const std::string& fontPath, const GLuint fontSize, const GLuint scrWidth, const GLuint scrHeight, const std::string& text)
{
	return std::shared_ptr<renderables::TextField>(new renderables::TextField(std::shared_ptr<SVFont>(new SVFont(quadVAO, quadVBO, fontPath, fontSize, scrWidth, scrHeight)), text));
}

/*static*/ std::shared_ptr<FontFactory> FontFactory::instance()
{
	if (!theInstance.get())
		theInstance.reset(new FontFactory());
	return theInstance;
}
