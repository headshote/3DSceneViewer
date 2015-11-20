#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2 vTextureCoord[];
in vec3 vNormal[];
in vec3 vWorldPosition[];

out vec2 theTextureCoord;
out vec3 theNormal;
out vec3 worldPosition;

uniform float time = 0;

vec3 getNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 2.0f;
    vec3 direction = normal * ((sin(time) + 1.0f) / 2.0f) * magnitude; 
    return position + vec4(direction, 0.0f);
} 

void main() 
{    
    vec3 normal = getNormal();

    gl_Position = explode(gl_in[0].gl_Position, normal);
	theNormal = vNormal[0];
	worldPosition = vWorldPosition[0];
    theTextureCoord = vTextureCoord[0];
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal);
	theNormal = vNormal[1];
	worldPosition = vWorldPosition[1];
    theTextureCoord = vTextureCoord[1];
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
	theNormal = vNormal[2];
	worldPosition = vWorldPosition[2];
    theTextureCoord = vTextureCoord[2];
    EmitVertex();

    EndPrimitive();
}  