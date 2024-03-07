#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include "stb_image.h"

#include <string>

#include "Shader.h"

class Texture
{
public:
	GLuint ID;
	GLenum texType;
	GLenum slot;
	std::string texMaterialType;
	std::string path;

	Texture(const char* path, GLenum texType, GLenum slot, GLenum format, GLenum pixelType, bool upsideDown);
	Texture(const char* path, const char* textType, GLenum slot);

	void texUnit(Shader& shader, const char* uniform, GLuint unit);
	void Bind();
	void Unbind();
	void Delete();
};
#endif // !TEXTURE_H

