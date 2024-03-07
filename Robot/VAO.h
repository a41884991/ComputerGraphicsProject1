#ifndef VAO_H
#define VAO_H

#include "VBO.h"

class VAO
{
public:
	GLuint ID;

	// constructor
	VAO();

	void LinkAttrib(VBO& vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);

	void Bind();

	void Unbind();

	void Delete();
};

#endif // !VAO_H
