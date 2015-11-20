#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoord;

out vec2 vTextureCoord;
out vec3 vNormal;
out vec3 vWorldPosition;

uniform mat4 model;
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