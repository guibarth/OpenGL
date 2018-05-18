#version 330 core

layout ( location = 0 ) in vec3 vPosition;
layout ( location = 1 ) in vec2 vMapping;
layout ( location = 2 ) in vec3 vNormals;

out vec3 v_Position;
out vec2 v_Mapping;
out vec3 v_Normals;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
//uniform mat4 camera;

void main()
{

// Pass some variables to the fragment shader
    v_Position = vPosition;
	v_Mapping = vMapping;
    v_Normals = vNormals;

	// Apply all matrix transformations to vert
    //gl_Position = view * model * vec4(vMapping, 1);


	gl_Position = projection * view * model * vec4(vPosition, 1.0);
	
	//gl_Position = camera * model * vec4(vPosition, 1);
	



	
    
    
}