#ifndef TEXT_FIELD
#define TEXT_FIELD

#include <string>
#include <memory>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderable.h"
#include "SVFont.h"

namespace renderables
{
	class TextField : public Renderable
	{
	public:
		TextField(std::shared_ptr<::textandfonts::SVFont> font, const std::string& text, const GLuint viewPortWidth, const GLuint viewPortHeight);
		~TextField();

		virtual GLuint getVAO();

		void setPosition(const GLfloat x, const GLfloat y);
		void setColor(const glm::vec3& color);
		void setRenderingDimensions(const GLuint width, const GLuint height);
		void setText(const std::string& newText);

	protected:
		virtual void render(const GLuint shaderprogram);
		virtual void batchRender(const GLuint shaderProgram, const GLuint numCalls);

	private:
		std::shared_ptr<::textandfonts::SVFont> tfFont;
		std::string tfText;

		GLuint scrWidth;
		GLuint scrHeight;

		GLfloat pX;
		GLfloat pY;

		glm::vec3 textColor;
	};
}

#endif