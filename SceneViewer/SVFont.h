#ifndef SV_FONT
#define SV_FONT

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H  

namespace textandfonts
{
	struct Character {
		GLuint     textureId;  // ID handle of the glyph texture
		glm::ivec2 size;       // Size of glyph
		glm::ivec2 bearing;    // Offset from baseline to left/top of glyph
		GLuint     advance;    // Offset to advance to next glyph
	};

	class SVFont
	{
	public:
		explicit SVFont(const GLuint VAO, const GLuint VBO);
		explicit SVFont(const GLuint VAO, const GLuint VBO, const GLchar* filePath);
		SVFont(const GLuint VAO, const GLuint VBO, const GLchar* filePath, const GLuint fontSize);
		SVFont(const GLuint VAO, const GLuint VBO, const GLchar* filePath, const GLuint fontSize, const GLuint scrWidth, const GLuint scrHeight);
		~SVFont();

		Character getCharData(std::string::const_iterator character);

		GLuint getVAO();
		GLuint getVBO();

		GLuint getScreenWidth();
		GLuint getScreenHeight();

	private:
		std::map<GLchar, Character> charMap;

		GLuint screenWidth;
		GLuint screenHeight;

		const GLuint quadVAO;
		const GLuint quadVBO;

		void loadTTFont(const GLchar* filePath = "fonts/arial.ttf", const GLuint fontSize = 48);
	};
}

#endif