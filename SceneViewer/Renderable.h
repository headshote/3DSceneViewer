#ifndef RENDERABLE
#define RENDERABLE

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace renderables
{
	class Renderable
	{
	public:
		virtual ~Renderable(){};

		virtual GLuint getVAO() = 0;

		void drawCall(const GLuint shederprogram);

		void drawBatch(const GLuint shederprogram, const GLuint numDrawCalls);

		void setRendMode(GLenum mode);

	protected:
		Renderable();

		GLenum rMode;

		virtual void render(const GLuint shaderprogram) = 0;
		virtual void batchRender(const GLuint shaderProgram, const GLuint numCalls) = 0;

	private:
		//no copy constructors and assignments
		Renderable(const Renderable&);
		Renderable& operator=(const Renderable&);
	};
}

#endif