#version 330 core
in vec4 fragmentPosition;

uniform vec3 lightPosition;
uniform float far_plane;

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(fragmentPosition.xyz - lightPosition);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // Write this as modified depth
    gl_FragDepth = lightDistance;
}  