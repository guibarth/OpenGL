#ifndef GROUP_H
#define GROUP_H
#endif


#include <GL\glew.h>

#include <string>
#include <vector>
#include "Shader.h"
#include "Face.h"
#include "Material.h"

enum GroupType
{
	NONE,
	EMPTY,
	V,
	V_T,
	V_N,
	V_T_N
};

class Group
{

public:

	Group();
	~Group();

	void SetName(std::string name);
	void SetMaterial(Material *material);

	void SetMaterialName(std::string name);
	std::string GetMaterialName();

	std::string GetName();
	Material* GetMaterial();

	void AddFace(std::vector<int>* vertexIndices, std::vector<int>* normalIndices, std::vector<int>* mappingIndices);
	std::vector<Face*>* GetFaces();

	GLint GetNumFaces();
	bool HasMaterial();

	void Bind(std::vector<GLfloat> *vertices);

	GLuint getVAO();
	GLuint getVBO();
	GroupType GetType();

	void SetType(GroupType type);

	void DefineType();
	void SetShader(Shader* s);

private:

	std::string name;
	Material *material = NULL;
	std::string nameMat;
	bool hasMaterial;
	GroupType type;

	std::vector<Face*>* faces;
	Shader* shaderObj;
	GLuint vao, vbo;


	int GetInputStride();

};
