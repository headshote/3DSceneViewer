#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;
uniform float exposure = 1.0f;

void main()
{ 
    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
  
    // Reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
  
    color = vec4(mapped, 1.0);
}