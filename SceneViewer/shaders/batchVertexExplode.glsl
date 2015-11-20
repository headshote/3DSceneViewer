#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoord;
layout (location = 3) in mat4 model;
//for normal maps
layout (location = 7) in vec3 tangent;
layout (location = 8) in vec3 bitangent;  

out vec2 vTextureCoord;
out vec3 vNormal;
out vec3 vWorldPosition;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
	gl_PointSize = 60.0 / gl_Position.z; 

	vWorldPosition = vec3( model * vec4(position, 1.0f) );

	vNormal = mat3(transpose(inverse(model))) * normal;

	vTextureCoord = textureCoord;
}