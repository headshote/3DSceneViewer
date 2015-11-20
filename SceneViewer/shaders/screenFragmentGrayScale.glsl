#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;
uniform sampler2D brightnessTexture;
uniform float exposure = 1.0f;

void main()
{ 
    color = texture(screenTexture, TexCoords);
	vec4 bloomColor = texture(brightnessTexture, TexCoords);

	color += bloomColor; // additive blending
    float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    color = vec4(average, average, average, 1.0);
	
    // Reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-color.rgb * exposure);
  
    color = vec4(mapped, 1.0);
}