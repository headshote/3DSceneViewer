#include "TextField.h"

using namespace renderables;

TextField::TextField(std::shared_ptr<::textandfonts::SVFont> font, const std::string& text, const GLuint viewPortWidth, const GLuint viewPortHeight) :
		tfFont(font), tfText(text), scrWidth(viewPortWidth), scrHeight(viewPortHeight), pX(0.f), pY(0.f), textColor(glm::vec3(1.0f, 1.0f, 1.0f))
{

}

TextField::~TextField()
{

}

GLuint TextField::getVAO()
{
	return tfFont->getVAO();
}

void TextField::setPosition(const GLfloat x, const GLfloat y)
{
	pX = x;
	pY = y;
}

void TextField::setColor(const glm::vec3& color)
{
	textColor = glm::vec3(color);
}

void TextField::setRenderingDimensions(const GLuint width, const GLuint height)
{
	scrWidth = width;
	scrHeight = height;
}

void TextField::setText(const std::string& newText)
{
	tfText = std::string(newText);
}

void TextField::render(const GLuint shaderprogram)
{
	GLuint quadVAO = tfFont->getVAO();
	GLuint quadVBO = tfFont->getVBO();

	glm::mat4 projection = glm::ortho(0.0f, (GLfloat)scrWidth, 0.0f, (GLfloat)scrHeight);

	glUseProgram(shaderprogram);

	glUniform3f(glGetUniformLocation(shaderprogram, "textColor"), textColor.x, textColor.y, textColor.z);
	glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(quadVAO);

	// Iterate through all characters
	std::string::const_iterator c;
	for (c = tfText.begin(); c != tfText.end(); c++)
	{
		textandfonts::Character ch = tfFont->getCharData(c);

		GLfloat xpos = pX + ch.bearing.x;
		GLfloat ypos = pY - (ch.size.y - ch.bearing.y);

		GLint w = ch.size.x;
		GLint h = ch.size.y;
		// Update VBO for each character
		GLfloat vertices[24] = {
			xpos, ypos + h, 0.0, 0.0,
			xpos, ypos, 0.0, 1.0,
			xpos + w, ypos, 1.0, 1.0,

			xpos, ypos + h, 0.0, 0.0,
			xpos + w, ypos, 1.0, 1.0,
			xpos + w, ypos + h, 1.0, 0.0
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureId);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		pX += (ch.advance >> 6); // divide by 64 using bitshift
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextField::batchRender(const GLuint shaderProgram, const GLuint numCalls)
{

}
