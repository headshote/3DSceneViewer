#version 330 core

out vec4 color;

uniform vec3 lsColor;

void main()
{	
    color = vec4(lsColor, 1.0f);
}
