#ifndef EBO_H
#define EBO_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

class EBO
{
public:
	GLuint ID;

	EBO(GLuint* indices, GLsizeiptr size);
	EBO(std::vector<GLuint> indices);

	void Bind();
	void Unbind();
	void Delete();
};
#endif // !EBO_H