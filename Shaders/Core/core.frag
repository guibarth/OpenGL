#version 330 core

in vec3 v_Position;

in vec2 v_Mapping;

in vec3 v_Normals;

out vec4 color;

uniform sampler2D texture1;

uniform mat4 model;

uniform struct Light {
  vec3 position;
  vec3 intensities; //a.k.a the color of the light
} light;



float near = 100.0; 
float far  = 100.0; 
  
float LinearizeDepth(float depth)
{
   float z = depth * 2.0 - 1.0; // back to NDC 
  return (2.0 * near * far) / (far + near - z * (far - near));	
}


/*void main()
{             
   float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration


	vec4 texel = texture(texture1, v_Mapping);
	
    color = vec4(vec3(depth), 1.0) * texel;

}*/

void main() {

	float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration

	

    //calculate normal in world coordinates
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 normal = normalize(normalMatrix * v_Normals);
    
    //calculate the location of this fragment (pixel) in world coordinates
    vec3 fragPosition = vec3(model * vec4(v_Position, 1));
    
    //calculate the vector from this pixels surface to the light source
    //vec3 surfaceToLight = light.position - fragPosition;
	vec3 surfaceToLight = vec3(-15.0f, 40.0f, 50.0f) - fragPosition;

    //calculate the cosine of the angle of incidence
    float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
    brightness = clamp(brightness, 0, 1);

    //calculate final color of the pixel, based on:
    // 1. The angle of incidence: brightness
    // 2. The color/intensities of the light: light.intensities
    // 3. The texture and texture coord: texture(texture1, v_Mapping)
    
	vec4 texel = texture(texture1, v_Mapping);
	
	color = vec4(vec3(depth), 1.0) * texel * texel.a * brightness;


    //color = vec4(brightness * light.intensities * texel.rgb, texel.a);
	}