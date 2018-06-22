#version 330 core

layout ( location = 0 ) in vec3 vertex_position;
layout ( location = 1 ) in vec2 vertex_mapping;
layout ( location = 2 ) in vec3 vertex_normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 position_eye;
out vec2 v_Mapping;
out vec3 normal_eye;



void main()
{

// Pass some variables to the fragment shader
    position_eye = vertex_position;
	v_Mapping = vertex_mapping;
    normal_eye = vertex_normal;

	position_eye = vec3 (view * model * vec4 (vertex_position, 1.0));
	normal_eye = vec3 (view * model * vec4 (vertex_normal, 0.0));
	
	
	gl_Position = projection * vec4 (position_eye, 1.0);
	//gl_Position = projection * view * model * vec4(position_eye, 1.0);    
    
}