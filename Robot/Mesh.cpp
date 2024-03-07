#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	this->bindAttrib();
}

//Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<MaterialAttributes>& materials) {
//	this->vertices = vertices;
//	this->indices = indices;
//	this->materials = materials;
//
//	this->bindAttrib();
//}



Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures, MaterialAttributes& material)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	this->material = material;

	this->bindAttrib();
}

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures, MaterialAttributes& material, std::string& name)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	this->material = material;
	this->name = name;

	this->bindAttrib();
}

void Mesh::Draw(Shader& shader) {
	shader.use();
	this->VAO.Bind();

	bool withTexture = !this->textures.empty();

	//if (withTexture) {

	//	//std::cout << "with Texture" << std::endl;
	//	unsigned int numDiffuse = 1;
	//	unsigned int numSpecular = 1;
	//	unsigned int numNormal = 1;
	//	unsigned int numHeight = 1;

	//	for (unsigned int i = 0; i < textures.size(); i++) {
	//		std::string num;
	//		std::string type = textures[i].texMaterialType;

	//		if (type == "texture_diffuse")
	//			num = std::to_string(numDiffuse++);
	//		else if (type == "texture_specular")
	//			num = std::to_string(numSpecular++);
	//		else if (type == "texture_normal")
	//			num = std::to_string(numNormal++);
	//		else if (type == "texture_height")
	//			num = std::to_string(numHeight++);

	//		textures[i].texUnit(shader, ("material." + type + num).c_str(), i);
	//		textures[i].Bind();
	//	}

	//}
	//else {
	shader.setVec3("material.ambient", material.ambient);
	shader.setVec3("material.diffuse", material.diffuse);
	shader.setVec3("material.specular", material.specular);
	shader.setFloat("material.shininess", material.shininess);
	//}

	this->VAO.Bind();
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	this->VAO.Unbind();
}

void Mesh::bindAttrib()
{
	this->VAO.Bind();
	VBO VBO(vertices);
	EBO EBO(indices);

	this->VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	this->VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	this->VAO.LinkAttrib(VBO, 2, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	this->VAO.LinkAttrib(VBO, 3, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	this->VAO.LinkAttrib(VBO, 4, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	this->VAO.LinkAttrib(VBO, 5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
	this->VAO.LinkAttrib(VBO, 6, 4, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

	this->VAO.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}

