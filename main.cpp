
#pragma once
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <sstream>
#include <stb_image.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <SOIL.h>
#include <GLM\glm.hpp>
#include <GLM\gtc\matrix_transform.hpp>
#include <GLM\gtc\type_ptr.hpp>
#include "Shader.h"
#include "OBJReader.h"
#include "MTLReader.h"
#include "Camera.h"

struct Light {
	glm::vec3 position;
	glm::vec3 intensities; //a.k.a. the color of the light
};

Light gLight;
tdogl::Camera gCamera;

int main() {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(800, 800, "OpenGL", nullptr, nullptr);
	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	if (window == nullptr) {
		std::cout << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		std::cout << "Failed no init GLEW." << std::endl;
	}

	glViewport(0, 0, screenWidth, screenHeight);

	Shader *coreShader = new Shader("Shaders/Core/core.vert", "Shaders/Core/core.frag");

	coreShader->Use();
		
	//read the necessary obj files to a vector
	std::vector<Mesh*>* meshVec = new std::vector<Mesh*>();
	std::string objs = "cenaPaintball.obj end";
	istringstream ss(objs);
	string temp;
	ss >> temp;
	while (temp != "end") {
		//Mesh * tempMesh = OBJReader::Read("paintball/cenaPaintball.obj");
		meshVec->push_back(OBJReader::Read(temp.c_str()));
		ss >> temp;
	}
	
	for (std::vector<Mesh*>::iterator obj = meshVec->begin(); obj != meshVec->end(); ++obj) {
		//read MTL files to the meshes
		(*obj)->setMaterials(MTLReader::read((*obj)->GetMeterialFile()));
	}

	for (std::vector<Mesh*>::iterator obj = meshVec->begin(); obj != meshVec->end(); ++obj){
		//print material list for all objs
		std::vector<Material*> *tempMats = (*obj)->GetMaterials();
		for (std::vector<Material*>::iterator mat = tempMats->begin(); mat != tempMats->end(); ++mat) {
			std::cout << (*mat)->GetName() << std::endl;
		}
	}	

	//assign materials to the groups within the meshes
	for (std::vector<Mesh*>::iterator obj = meshVec->begin(); obj != meshVec->end(); ++obj) {
						
		std::vector<Group*> *tempGroups = (*obj)->GetGroups();
		std::vector<Material*> *tempMaterials = (*obj)->GetMaterials();
		std::string name;
		//iterate through the groups and add the materials to them
		for (std::vector<Group*>::iterator it = tempGroups->begin(); it != tempGroups->end(); ++it) {
			//set shader on the group
			(*it)->SetShader(coreShader);
			for (std::vector<Material*>::iterator itMaterial = tempMaterials->begin(); itMaterial != tempMaterials->end(); ++itMaterial) {
				if ((*it)->GetMaterialName() == (*itMaterial)->GetName()) {
					Material *newMat = new Material((*itMaterial)->GetName());
					(*it)->SetMaterial((*itMaterial));
				}
			}
		}
	}

	//bind all the meshes
	for (std::vector<Mesh*>::iterator obj = meshVec->begin(); obj != meshVec->end(); ++obj) {
		(*obj)->Bind();
	}

		
	glm::mat4 model(1.0f);
	int modelLoc = coreShader->Uniform("model");
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	glm::mat4 view(1.0f);
	int viewLoc = coreShader->Uniform("view");
	view = glm::translate(view, glm::vec3(0.0f, -5.0f, -70.0f));

	glm::mat4 projection(1.0f);
	int projLoc = coreShader->Uniform("projection");
	projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

	float angle = 0.0f;
	
	//setup gLight

	//gLight.position = gCamera.position();
	gLight.position = glm::vec3(0.0f, 1.0f, -30.0f);
	gLight.intensities = glm::vec3(1, 1, 1); //white

	glm::mat4 camera(1.0f);
	int camLoc = coreShader->Uniform("camera");
	camera = gCamera.matrix();

	glm::vec3 position(1.0f);
	int positionLoc = coreShader->Uniform("light.position");
	position = gLight.position;

	glm::vec3 intensities(1.0f);
	int intensitiesLoc = coreShader->Uniform("light.intensities");
	intensities = gLight.intensities;

	//texture
	stbi_set_flip_vertically_on_load(true);
	
	// setup gCamera
	gCamera.setPosition(glm::vec3(-4, 0, 17));
	gCamera.setViewportAspectRatio(800 / 800);
	gCamera.setNearAndFarPlanes(0.5f, 100.0f);

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		if (glfwGetKey(window, 'A') == GLFW_PRESS) {
			view = glm::translate(view, glm::vec3(0.06f, 0.0f, 0.0f));
		}
		if (glfwGetKey(window, 'D') == GLFW_PRESS) {
			view = glm::translate(view, glm::vec3(-0.06f, 0.0f, 0.0f));
		}

		if (glfwGetKey(window, 'W') == GLFW_PRESS) {
			view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.06f));
		}
		if (glfwGetKey(window, 'S') == GLFW_PRESS) {
			view = glm::translate(view, glm::vec3(0.0f, 0.0f, -0.06f));
		}

		if (glfwGetKey(window, 'X') == GLFW_PRESS) {
			view = glm::translate(view, glm::vec3(0.0f, 0.06f, 0.0f));
		}
		if (glfwGetKey(window, 'Z') == GLFW_PRESS) {
			view = glm::translate(view, glm::vec3(0.0f, -0.06f, 0.0f));
		}



		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		coreShader->Use();

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glUniformMatrix4fv(camLoc, 1, GL_FALSE, glm::value_ptr(camera));

		glUniformMatrix4fv(positionLoc, 1, GL_FALSE, glm::value_ptr(position));
		glUniformMatrix4fv(intensitiesLoc, 1, GL_FALSE, glm::value_ptr(intensities));

		std::vector<Group*>* currentGroups = nullptr;

		//iterate through the different meshes
		for (std::vector<Mesh*>::iterator obj = meshVec->begin(); obj != meshVec->end(); ++obj) {
			currentGroups = (*obj)->GetGroups();

			// Iterare through groups
			for (std::vector<Group*>::iterator group = currentGroups->begin(); group != currentGroups->end(); ++group) {
				if ((*group)->GetType() != GroupType::EMPTY && (*group)->GetType() != GroupType::NONE) {
					glBindVertexArray((*group)->getVAO());

					int textureLocation = coreShader->Uniform("texture1");
					glEnable(GL_TEXTURE_2D);
					if ((*group)->HasMaterial()) {
						Material *mat = (*group)->GetMaterial();
						if (mat->GetHasTexture()) {
							int textureId = mat->getTextureId();

							glUniform1i(textureLocation, textureId);
							//glActiveTexture(GL_TEXTURE0);
							//glEnable(GL_TEXTURE_2D);						
							glBindTexture(GL_TEXTURE_2D, textureId);

						}
					}
					glDrawArrays(GL_TRIANGLES, 0, (*group)->GetNumFaces() * 3);
					glDisable(GL_TEXTURE_2D);
				}
			}
		}
		
		glfwSwapBuffers(window);
	}
	coreShader->Delete();
	glfwTerminate();
	return 0;
}
#endif
