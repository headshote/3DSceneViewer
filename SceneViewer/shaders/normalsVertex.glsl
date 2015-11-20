#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureCoord;
//for normal maps
layout (location = 7) in vec3 tangent;
layout (location = 8) in vec3 bitangent;  

out VS_OUT {
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); 
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));

    vs_out.normal = normalize(vec3(projection * vec4(normalMatrix * normal, 1.0)));
    vs_out.tangent = normalize(vec3(projection * vec4(normalMatrix * tangent, 1.0)));
    vs_out.bitangent = normalize(vec3(projection * vec4(normalMatrix * bitangent, 1.0)));
}