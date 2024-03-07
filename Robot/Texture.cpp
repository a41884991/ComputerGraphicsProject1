#include "Texture.h"

Texture::Texture(const char* path, GLenum texType, GLenum slot, GLenum format, GLenum pixelType, bool upsideDown)
{
	this->texType = texType;
	this->slot = slot;
	this->texMaterialType = "";
	this->path = path;

	stbi_set_flip_vertically_on_load(upsideDown);
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	//產生texture並綁定
	glGenTextures(1, &this->ID);
	glActiveTexture(slot);
	glBindTexture(texType, this->ID);

	//設定texture格式
	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//輸入texture資料並產生mipmap
	if (data)
	{
		glTexImage2D(texType, 0, GL_RGBA, width, height, 0, format, pixelType, data);
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) {
			printf("texture error\n");
		}
		glGenerateMipmap(texType);
	}
	else
	{
		printf("load texture error\n");
	}

	//釋放image資源
	stbi_image_free(data);
	//解綁以免意外更動
	glBindTexture(this->texType, 0);
}

Texture::Texture(const char* path, const char* texMaterialType, GLenum slot)
{
	this->texType = GL_TEXTURE_2D;
	this->slot = slot;
	this->texMaterialType = texMaterialType;

	size_t lastSlashPos = std::string(path).find_last_of('/');
	if (lastSlashPos != std::string::npos)
		this->path = std::string(path).substr(lastSlashPos + 1);
	else
		this->path = path;

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	glGenTextures(1, &this->ID);
	glActiveTexture(slot);
	glBindTexture(this->texType, this->ID);

	//設定texture格式
	glTexParameteri(this->texType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(this->texType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(this->texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(this->texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLenum format = NULL;

	switch (nrChannels)
	{
	case 1:
		format = GL_RED;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	}

	//輸入texture資料並產生mipmap
	if (data && format)
	{
		glTexImage2D(this->texType, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		GLenum error = glGetError();
		if (error != GL_NO_ERROR) 
		{
			std::cout << "TEXTURE::FAIL::" << this->path << std::endl;
		}
		glGenerateMipmap(this->texType);
	}
	else
	{
		std::cout << "LOAD::TEXTURE::FAIL" << std::endl;
	}

	//釋放image資源
	stbi_image_free(data);
	//解綁以免意外更動
	glBindTexture(this->texType, 0);
}

void Texture::texUnit(Shader& shader, const char* uniform, GLuint unit)
{
	GLuint texUniform = glGetUniformLocation(shader.ID, uniform);
	this->slot = GL_TEXTURE0 + unit;

	shader.use();
	glUniform1i(texUniform, unit);
}

void Texture::Bind()
{
	glActiveTexture(this->slot);
	glBindTexture(this->texType, this->ID);
}

void Texture::Unbind()
{
	glBindTexture(this->texType, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &this->ID);
}
