#ifndef OBJREADER_H
#define OBJREADER_H

#include <GL\glew.h>
#include <glm\vec3.hpp>
#include <glm\vec2.hpp>
#include <string>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>

#include "Mesh.h"
#include <map>

namespace OBJReader {

	enum tokenType {
		NONE,
		V,
		VN,
		VT,
		F,
		G,
		MTLLIB,
		COMMENTARY,
		USEMTL
	};

	typedef std::map<std::string, tokenType> TypesMap;
	static TypesMap typesMap = {
		{ "v", tokenType::V },
	{ "vn", tokenType::VN },
	{ "vt", tokenType::VT },
	{ "f", tokenType::F },
	{ "g", tokenType::G },
	{ "mtllib", tokenType::MTLLIB },
	{ "#", tokenType::COMMENTARY },
	{ "usemtl", tokenType::USEMTL }
	};

	static Mesh* Read(const GLchar* path) {

		Mesh* mesh = new Mesh();

		std::ifstream file;
		file.exceptions(std::ifstream::badbit);

		Group* currentGroup = nullptr;

		std::string mtl_file;
		std::string group_mtl;

		try {
			file.open(path);

			if (!file.is_open()) {
				std::cout << "ERROR::OBJREADER::PATH ERROR";
			}

			std::string line, temp;
			std::stringstream sstream;
			int lineCounter = 1;

			while (!file.eof()) {

				sstream = std::stringstream();
				line = temp = "";

				//get first line of the file
				std::getline(file, line);

				//get content of the line
				sstream << line;
				sstream >> temp;

				//get initial token (g, f, etc)
				tokenType type = typesMap[temp];

				switch (type)
				{
				case OBJReader::V:
				{
					float x, y, z;
					sstream >> x >> y >> z;
					mesh->AddVertex(glm::vec3(x, y, z));
					break;
				}
				case OBJReader::VN:
				{
					float x, y, z;
					sstream >> x >> y >> z;
					mesh->AddNormal(glm::vec3(x, y, z));
					break;
				}
				case OBJReader::VT:
				{
					float x, y;
					sstream >> x >> y;
					//test to invert v = 1 - v to treat objs created on blender
					y = 1.0 - y;
					mesh->AddMapping(glm::vec2(x, y));
					break;
				}
				case OBJReader::F:
				{
					//create vectors for each variable
					std::vector<int> *posV = new std::vector<int>;
					std::vector<int> *normV = new std::vector<int>;
					std::vector<int> *texV = new std::vector<int>;
					std::string token, valueAux;
					std::stringstream tokenStream;

					//while there is content on the line
					while (sstream.rdbuf()->in_avail()) {

						token = valueAux = "";
						sstream >> token;

						tokenStream = std::stringstream();
						tokenStream << token;

						//on every loop, one subset of the data is analyzed as the f may come on blocks of three (f x1/y1/z1 x2/y2/z2 x3/y3/z3)
						if (token.find('/') == std::string::npos) { // in case it is only vertices, they are separated by spaces f x y z so there is no /
																	//std::string.find() returns string::npos in case it does not find any match
							while (tokenStream.rdbuf()->in_avail()) {
								std::getline(tokenStream, valueAux, ' ');
								posV->push_back(std::atoi(&valueAux[0]));
							}
						}
						else {

							while (tokenStream.rdbuf()->in_avail()) {
								std::getline(tokenStream, valueAux, '/');

								posV->push_back(std::atoi(&valueAux[0]));

								std::getline(tokenStream, valueAux, '/');
								if (valueAux == "") { // v/n
									std::getline(tokenStream, valueAux, '/');
									normV->push_back(std::atoi(&valueAux[0]));
								}
								else {
									texV->push_back(std::atoi(&valueAux[0])); // v/t

									std::getline(tokenStream, valueAux, '/');
									if (valueAux != "") {
										normV->push_back(std::atoi(&valueAux[0])); // v/t/n
									}
								}
							}
						}

					}

					if (currentGroup != nullptr) {
						currentGroup->AddFace(posV, normV, texV);
					}
					else {
						std::cout << "ERROR::OBJREADER::NO GROUP DEFINED FOR FACE AT LINE " << lineCounter << std::endl;

						bool addedGroup = false;
						std::vector<Group*> *tempGroups = mesh->GetGroups();
						for (std::vector<Group*>::iterator it = tempGroups->begin(); it != tempGroups->end(); ++it) {
							if ("NO_GROUP" == (*it)->GetName()) { // Checking if a group with that name already exists
								currentGroup = *it;
								addedGroup = true;
								break;
							}
						}
						if (!addedGroup) {
							mesh->AddGroup("NO_GROUP");
							currentGroup = mesh->GetGroups()->back();
						}
					}
					break;
				}
				case OBJReader::G:
				{
					std::string name;
					std::getline(sstream, name);

					bool addGroup = true;
					std::vector<Group*> *tempGroups = mesh->GetGroups();
					for (std::vector<Group*>::iterator it = tempGroups->begin(); it != tempGroups->end(); ++it) {
						if (name == (*it)->GetName()) { // Checking if a group with that name already exists
							addGroup = false;
							currentGroup = *it;
							break;
						}
					}

					if (addGroup) { //if a group with such name does not exist yet, create it
						mesh->AddGroup(name);
						currentGroup = mesh->GetGroups()->back();
					}
					break;
				}
				case OBJReader::MTLLIB: //save the mtlfile on the mesh		
					std::getline(sstream, mtl_file);
					mesh->setMaterialFile(mtl_file.substr(1, std::string::npos));
					break;
				case OBJReader::USEMTL: //save the material of the group to be used from the mtlfile
					std::getline(sstream, group_mtl);
					group_mtl = group_mtl.substr(1, std::string::npos);
					currentGroup->SetMaterialName(group_mtl);
					//std::cout << currentGroup->GetName();
					//std::cout << currentGroup->GetMaterial();
				case OBJReader::COMMENTARY:
				case OBJReader::NONE:
					break;
				default:
					std::cout << "Not a valid line at " << lineCounter << std::endl;
					break;
				}
				lineCounter++;
			}

			file.close();
			return mesh;
		}
		catch (const std::ifstream::failure& e) {

			if (!file.eof()) {
				std::cout << "ERROR::OBJREADER::FILE NOT SUCCESUFULLY READ" << std::endl;
			}

		}
		file.close();
		return nullptr;
	}
};
#endif