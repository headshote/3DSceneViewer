#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;
uniform float exposure = 1.0f;

uniform float screenWidth = 1280.0f;

void main()
{ 
	vec4 textureColor = texture(screenTexture, TexCoords);
	if(	gl_FragCoord.x < screenWidth * 0.5 )
		color = vec4(textureColor.r+0.1, textureColor.g-0.3, textureColor.b+0.2, textureColor.a);
	else
		color = vec4(textureColor.r+0.2, textureColor.g+0.1, textureColor.b-0.3, textureColor.a);

    // Reinhard tone mapping
    vec3 mapped = vec3(1.0) - exp(-color.rgb * exposure);
  
    color = vec4(mapped, 1.0);
}