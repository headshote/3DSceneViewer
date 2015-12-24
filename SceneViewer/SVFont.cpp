#include "SVFont.h"

using namespace textandfonts;

SVFont::SVFont(const GLuint VAO, const GLuint VBO) : quadVAO(VAO), quadVBO(VBO)
{
	loadTTFont();
}

SVFont::SVFont(const GLuint VAO, const GLuint VBO, const std::string& filePath) :
	screenWidth(1280), screenHeight(720), quadVAO(VAO), quadVBO(VBO)
{
	loadTTFont(filePath);
}

SVFont::SVFont(const GLuint VAO, const GLuint VBO, const std::string& filePath, const GLuint fontSize) :
	screenWidth(1280), screenHeight(720), quadVAO(VAO), quadVBO(VBO)
{
	loadTTFont(filePath, fontSize);
}

SVFont::SVFont(const GLuint VAO, const GLuint VBO, const std::string& filePath, const GLuint fontSize, const GLuint scrWidth, const GLuint scrHeight) :
	screenWidth(scrWidth), screenHeight(scrHeight), quadVAO(VAO), quadVBO(VBO)
{
	loadTTFont(filePath, fontSize);
}

SVFont::~SVFont()
{
	for (std::map<GLchar, Character>::const_iterator iter = charMap.cbegin(); iter != charMap.cend(); ++iter)
	{
		std::pair<GLchar, Character> characterPair = *iter;
		Character character = characterPair.second;
		glDeleteTextures(1, &character.textureId);
	}
}

void SVFont::loadTTFont(const std::string& filePath, const GLuint fontSize)
{
	charMap;

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	FT_Face face;
	if (FT_New_Face(ft, filePath.c_str(), 0, &face))
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

	//The function sets the font's width and height parameters. 
	//Setting the width to 0 lets the face dynamically calculate the width based on the given height. 
	FT_Set_Pixel_Sizes(face, 0, fontSize);

	//Pre-laod 128 chars to opengl textures
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
			);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		charMap.insert(std::pair<GLchar, Character>(c, character));
	}

	//done, release resources
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

GLuint SVFont::getVAO()
{
	return quadVAO;
}

GLuint SVFont::getVBO()
{
	return quadVBO;
}

GLuint SVFont::getScreenWidth()
{
	return screenWidth;
}

GLuint SVFont::getScreenHeight()
{
	return screenHeight;
}

Character SVFont::getCharData(std::string::const_iterator character)
{
	return charMap[*character];
}