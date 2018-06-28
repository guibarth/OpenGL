#include <sstream>
#include <stb_image.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <GLM\glm.hpp>
#include <GLM\gtc\matrix_transform.hpp>
#include <GLM\gtc\type_ptr.hpp>
#include "Shader.h"
#include "OBJReader.h"
#include "MTLReader.h"
#include "Camera.h"

int textureNum = 0;
float scaleFactor = 45.0f;

std::vector<glm::vec3*>* curvePoints = new std::vector<glm::vec3*>();
std::vector<glm::vec3*>* scaledCurvePoints = new std::vector<glm::vec3*>();

void readCurvePoints(const GLchar* path);
void scaleCurvePoints(std::vector<glm::vec3*>* points, float factor);
float calculateAngle(int indexA, int indexB);
//float getInitialAngleSync(glm::vec3* curvePoint);

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

	// Read and scale curve
	readCurvePoints("originalCurve.txt");
	scaleCurvePoints(curvePoints, scaleFactor);

	// Read other OBJ files
	std::vector<Mesh*>* meshVec = new std::vector<Mesh*>();
	std::string objs = "car.obj curve.obj end";
	istringstream ss(objs);
	string temp;
	ss >> temp;
	while (temp != "end") {
		meshVec->push_back(OBJReader::Read(temp.c_str()));
		ss >> temp;
	}

	for (std::vector<Mesh*>::iterator obj = meshVec->begin(); obj != meshVec->end(); ++obj) {
		// Read MTL files to the meshes
		(*obj)->setMaterials(MTLReader::read((*obj)->GetMeterialFile(), textureNum));
	}

	for (std::vector<Mesh*>::iterator obj = meshVec->begin(); obj != meshVec->end(); ++obj) {
		// Print material list for all objs
		std::vector<Material*> *tempMats = (*obj)->GetMaterials();
		for (std::vector<Material*>::iterator mat = tempMats->begin(); mat != tempMats->end(); ++mat) {
			std::cout << (*mat)->GetName() << std::endl;
		}
	}

	// Assign materials to the groups within the meshes
	for (std::vector<Mesh*>::iterator obj = meshVec->begin(); obj != meshVec->end(); ++obj) {

		std::vector<Group*> *tempGroups = (*obj)->GetGroups();
		std::vector<Material*> *tempMaterials = (*obj)->GetMaterials();
		std::string name;
		// Iterate through the groups and add the materials to them
		for (std::vector<Group*>::iterator it = tempGroups->begin(); it != tempGroups->end(); ++it) {
			// Set shader on the group
			(*it)->SetShader(coreShader);
			for (std::vector<Material*>::iterator itMaterial = tempMaterials->begin(); itMaterial != tempMaterials->end(); ++itMaterial) {
				if ((*it)->GetMaterialName() == (*itMaterial)->GetName()) {
					Material *newMat = new Material((*itMaterial)->GetName());
					(*it)->SetMaterial((*itMaterial));
				}
			}
		}
	}

	// Bind all the meshes
	for (std::vector<Mesh*>::iterator obj = meshVec->begin(); obj != meshVec->end(); ++obj) {
		(*obj)->Bind();
	}

	glm::mat4 model(1.0f);
	int modelLoc = coreShader->Uniform("model");
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 view(1.0f);
	int viewLoc = coreShader->Uniform("view");
	view = glm::translate(view, glm::vec3(0.0f, -5.0f, -70.0f));

	glm::mat4 projection(1.0f);
	int projLoc = coreShader->Uniform("projection");
	projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);

	//float angSync = getInitialAngleSync(curvePoints->at(0));
	float angle = 0.0f;
	int movementIndex = 0;

	while (!glfwWindowShouldClose(window)) {

		if (movementIndex == curvePoints->size() - 10) {
			movementIndex = 0;
		}

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
		if (glfwGetKey(window, 'Q') == GLFW_PRESS) {
			angle += 1.0f;
			view = glm::rotate(view, glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (glfwGetKey(window, 'E') == GLFW_PRESS) {
			angle -= 1.0f;
			view = glm::rotate(view, glm::radians(-1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}

		glClearColor(0.5f, 0.8f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		coreShader->Use();

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		std::vector<Group*>* currentGroups = nullptr;

		// Iterate through Meshes
		for (std::vector<Mesh*>::iterator obj = meshVec->begin(); obj != meshVec->end(); ++obj) {
			currentGroups = (*obj)->GetGroups();

			// Iterare through Groups
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
							glBindTexture(GL_TEXTURE_2D, textureId);
						}

						if ((*group)->GetName() == "road") {
							glm::mat4 transform = glm::scale(model, glm::vec3(scaleFactor));
							glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transform));
						}
						else {
							glm::mat4 transform = glm::translate(model, glm::vec3(curvePoints->at(movementIndex)->x, curvePoints->at(movementIndex)->y, curvePoints->at(movementIndex)->z));
							angle = -calculateAngle(movementIndex, movementIndex + 5);
							angle += 4.71239f; // Add 270º to fix initial direction from the Car							
							transform = glm::rotate(transform, angle, glm::vec3(0, 1, 0));
							glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(transform));
						}
					}

					glDrawArrays(GL_TRIANGLES, 0, (*group)->GetNumFaces() * 3);
					glDisable(GL_TEXTURE_2D);
				}
			}
		}

		movementIndex += 5;
		glfwSwapBuffers(window);
	}
	coreShader->Delete();
	glfwTerminate();
	return 0;
}

void readCurvePoints(const GLchar* path) {

	std::ifstream file;
	file.exceptions(std::ifstream::badbit);

	try {
		file.open(path);

		if (!file.is_open()) {
			std::cout << "ERROR::Curve Points::PATH ERROR";
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

			if (temp == "v") {
				float x, y, z;
				sstream >> x >> y >> z;
				curvePoints->push_back(new glm::vec3(x, y, z));
			}
			lineCounter++;
		}
		file.close();
	}
	catch (const std::ifstream::failure& e) {
		if (!file.eof()) {
			std::cout << "ERROR::Curve Points::FILE NOT SUCCESUFULLY READ" << std::endl;
		}
	}
}

void scaleCurvePoints(std::vector<glm::vec3*>* points, float factor) {
	for (int i = 0; i < points->size(); i++) {
		scaledCurvePoints->push_back(new glm::vec3(points->at(i)->x*factor, points->at(i)->y, points->at(i)->z*factor));
	}
	curvePoints = scaledCurvePoints;
}

float calculateAngle(int indexA, int indexB) {


	glm::vec3* a = curvePoints->at(indexA);
	glm::vec3* b;

	if (indexA == curvePoints->size() - 5) {
		b = curvePoints->at(0);
	}
	else {
		b = curvePoints->at(indexB);
	}

	GLfloat dx = b->x - a->x;
	GLfloat dz = b->z - a->z;

	GLfloat angle = glm::atan(dz, dx);

	return angle;
}

/*
float getInitialAngleSync(glm::vec3* curvePoint) {
	float ang = 0;
	float dx = curvePoint->x - 0.0f; //car is on zero
	float dy = 1.0f - curvePoint->y;
	ang = glm::atan(dx, dy);
	return ang;
}
*/