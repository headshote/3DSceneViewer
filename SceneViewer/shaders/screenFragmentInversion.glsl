#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;
uniform float exposure = 1.0f;

void main()
{ 
   color = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
   
    // Reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-color.rgb * exposure);
  
    color = vec4(mapped, 1.0);
}