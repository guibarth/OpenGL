
#pragma once
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION


#include <stb_image.h>

#include "Group.h"
#include <stdlib.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

Group::Group()
{
	faces = new std::vector<Face*>();
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	type = NONE;
	this->hasMaterial = false;
}

Group::~Group() {}



void Group::SetName(std::string name) {
	this->name = name;
};
void Group::SetMaterial(Material *material) {
	this->material = material;
	this->hasMaterial = true;
};

bool Group::HasMaterial() {
	if (this->hasMaterial)
		return true;
	else
		return false;
}

void Group::SetMaterialName(std::string name) {
	this->nameMat = name;
}

std::string Group::GetMaterialName() { return this->nameMat; }

std::string Group::GetName() {
	return name;
};
Material* Group::GetMaterial() {
	return material;
};

std::vector<Face*>* Group::GetFaces() {
	return faces;
}

GLint Group::GetNumFaces() {
	return faces->size();
}

GLuint Group::getVAO() {
	return vbo;
}
GLuint Group::getVBO() {
	return vbo;
}

GroupType Group::GetType() {
	return type;
}

void Group::SetShader(Shader* s) { shaderObj = s; }

void Group::SetType(GroupType type) {
	this->type = type;
}

void Group::AddFace(std::vector<int>* vertexIndices, std::vector<int>* normalIndices, std::vector<int>* mappingIndices)
{
	Face* newFace = new Face(vertexIndices, normalIndices, mappingIndices);
	this->faces->push_back(newFace);
}

void Group::Bind(std::vector<GLfloat> *vertices){
	//texture is only set if the group has a material
	if (this->HasMaterial()) {
		if (this->GetMaterial()->GetHasTexture()) { //texture is only set if the material has a texture
			//create the texture
			unsigned int texture;
			glGenTextures(1, &texture);
			int textureId = this->GetMaterial()->getTextureId();
			
			int textureLocation = shaderObj->Uniform("texture1");
			glUniform1i(textureLocation, textureId);
			
			
			
			glActiveTexture(GL_TEXTURE0 + textureId);
			
			glBindTexture(GL_TEXTURE_2D, textureId);			

			//texture-space treatment
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			
			//enable
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);

			int width, height, nrChannels;

			//load image into program
			std::string textureName = this->GetMaterial()->GetMapKd();
			char *texture_name = &textureName[0];
			unsigned char *img = stbi_load(texture_name, &width, &height, &nrChannels, 0);

			if (img) {
				std::cout << "in the loop" << std::endl;
				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT , GL_UNSIGNED_BYTE, img);
				//the line below works for .png files
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
				//the line below works for .jpg files
				//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
				std::cout << "Texture image loaded successfully" << std::endl;								
			}

			
			//generate mipmap
			glGenerateMipmap(GL_TEXTURE_2D);

			/*
			glUniform1i(this->shaderObj->Uniform("texture1"), textureId);
			glBindTexture(GL_TEXTURE_2D, 0);
			*/
			//free memory
			stbi_image_free(img);
		}		
	}
	
	
	if (type == NONE) {
		DefineType();
	}

	if (type != GroupType::EMPTY) {

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices->size(), &vertices->at(0), GL_STATIC_DRAW);

		int stride = GetInputStride();
		int points = 0;

		//Vertexes
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat), (GLvoid *)0);
		glEnableVertexAttribArray(0);
		points += 3;

		if (type == GroupType::V_T || type == GroupType::V_T_N) {
			//Texture
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat), (GLvoid *)(points * sizeof(GLfloat)));
			/*
			Properties of VertexAttribPointer:
			1 - layout on shader
			2 - how many numbers to read
			3 - GL_FLOAT
			4 - GL_FALSE
			5 - interval of numbers from each number to be read, stride determines that
			6 - where to start, points is based on the type of the group (V, VT, VTN...)
			*/
			glEnableVertexAttribArray(1);
			points += 2;
		}

		if (type == GroupType::V_N || type == GroupType::V_T_N) {
			//Normals
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride * sizeof(GLfloat), (GLvoid *)(points * sizeof(GLfloat)));
			glEnableVertexAttribArray(2);
		}
		glBindVertexArray(0);
	}

}

void Group::DefineType()
{
	auto tV = faces->at(0)->GetVerts()->size();
	auto tN = faces->at(0)->GetNorms()->size();
	auto tT = faces->at(0)->GetTexts()->size();

	if (tV == 0) {
		type = EMPTY;
	}
	else {
		if (tT > 0) { //has texture
			if (tN > 0) { //has normals
				type = GroupType::V_T_N;
			}
			else {
				type = GroupType::V_T;
			}
		}
		else { //does not have texture
			if (tN > 0) { //has normals
				type = GroupType::V_N;
			}
			else { //does not have normals
				type = GroupType::V;
			}
		}
	}
}

int Group::GetInputStride() {
	if (type == V) {
		return 3;
	}
	else if (type == V_N) {
		return 5;
	}
	else if (type == V_T) {
		return 4;
	}
	else if (type == V_T_N) {
		return 8;
	}
}
#endif