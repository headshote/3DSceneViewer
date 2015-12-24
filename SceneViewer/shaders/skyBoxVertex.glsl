#version 330 core
layout (location = 0) in vec3 position;
out vec3 textureCoordinates;

uniform mat4 projection;
uniform mat4 pinnedView;


void main()
{
	vec4 pos = projection * pinnedView * vec4(position, 1.0);

	//The resulting normalized device coordinates will then always have a z value equal to 1.0: the maximum depth value. 
	//The skybox will as a result only be rendered wherever there are no objects visible
	// (only then it will pass the depth test, everything else is in front of the skybox). 
    gl_Position = pos.xyww;

    textureCoordinates = position;
}