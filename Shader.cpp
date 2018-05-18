#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

Shader::~Shader()
{
}

GLint Shader::Attribute(const GLchar* name)
{
	return glGetAttribLocation(program, name);
}

GLint Shader::Uniform(const GLchar * name)
{
	return glGetUniformLocation(program, name);
}
