#version 330 core
in vec3 textureCoordinates;
layout (location = 0) out vec4 color;
layout (location = 1) out vec4 brightRegions;

uniform samplerCube skybox;

void main()
{    
    color = texture(skybox, textureCoordinates);

	// Check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        brightRegions = vec4(color.rgb, 1.0);
}