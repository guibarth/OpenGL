#version 330 core

in vec3 position_eye;
in vec2 v_Mapping;
in vec3 normal_eye;

uniform mat4 view;

uniform vec3 ks;
uniform vec3 kd;
uniform vec3 ka;

// fixed point light properties
vec3 light_position_world  = vec3 (50.0, 5.0, -50.0);
vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
vec3 Ld = vec3 (3.0, 3.0, 3.0); // dull white diffuse light colour
vec3 La = vec3 (0.2, 0.2, 0.2); // grey ambient colour
  
// surface reflectance
//vec3 Ks = vec3 (1.0, 1.0, 1.0); // fully reflect specular light
//vec3 Kd = vec3 (1.0, 0.5, 0.0); // orange diffuse surface reflectance
//vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light
vec3 Ks = ks; // fully reflect specular light
vec3 Kd = kd; // orange diffuse surface reflectance
vec3 Ka = ka; // fully reflect ambient light
float specular_exponent = 100.0; // specular 'power'

out vec4 fragment_colour;

uniform sampler2D texture1;
uniform mat4 model;
  
void main () {
	// ambient intensity
	vec3 Ia = La * Ka;

	// normalize in case interpolation has upset normals' lengths
	vec3 n_eye = normalize( normal_eye );

	// diffuse intensity
	// raise light position to eye space
	vec3 light_position_eye = vec3 (view * vec4 (light_position_world, 1.0));
	vec3 distance_to_light_eye = light_position_eye - position_eye;
	vec3 direction_to_light_eye = normalize (distance_to_light_eye);
	float dot_prod = dot (direction_to_light_eye, n_eye);
	dot_prod = max (dot_prod, 0.0);
	vec3 Id = Ld * Kd * dot_prod; // final diffuse intensity
	
	// specular intensity
	vec3 surface_to_viewer_eye = normalize (-position_eye);
	
	//vec3 reflection_eye = reflect (-direction_to_light_eye, n_eye);
	//float dot_prod_specular = dot (reflection_eye, surface_to_viewer_eye);
	//dot_prod_specular = max (dot_prod_specular, 0.0);
	//float specular_factor = pow (dot_prod_specular, specular_exponent);
	
	// blinn
	vec3 half_way_eye = normalize (surface_to_viewer_eye + direction_to_light_eye);
	float dot_prod_specular = max (dot (half_way_eye, n_eye), 0.0);
	float specular_factor = pow (dot_prod_specular, specular_exponent);
	
	vec3 Is = Ls * Ks * specular_factor; // final specular intensity
	
	// final colour
	fragment_colour = vec4 (Is + Id + Ia, 1.0);

	//Texture
	vec4 texel = texture(texture1, v_Mapping);
	fragment_colour = fragment_colour * texel * texel.a;
}