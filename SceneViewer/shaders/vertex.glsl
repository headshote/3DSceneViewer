#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoord;
//for normal maps
layout (location = 7) in vec3 tangent;
layout (location = 8) in vec3 bitangent;  

out vec2 theTextureCoord;
out vec3 theNormal;
out vec3 worldPosition;
out vec4 lightSpacePosition;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
	gl_PointSize = 60.0 / gl_Position.z; 

	worldPosition = vec3( model * vec4(position, 1.0f) );
	
	lightSpacePosition = lightSpaceMatrix * vec4(worldPosition, 1.0);

	theNormal = mat3(transpose(inverse(model))) * normal;
	
	vec3 T = normalize(vec3(model * vec4(tangent,   0.0)));
	vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));   
	vec3 N = normalize(vec3(model * vec4(normal,    0.0)));
    TBN = mat3(T, B, N);

	theTextureCoord = textureCoord;
}