#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 18) out;

in VS_OUT {
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
} gs_in[];

out vec4 lineColor;

const float MAGNITUDE = 0.1f;

void GenerateLines(int index)
{
	//normal
	lineColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    gl_Position = gl_in[index].gl_Position;
    EmitVertex();
	lineColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
    gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0f) * MAGNITUDE;
    EmitVertex();
    EndPrimitive();

	//tang	
	lineColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
    gl_Position = gl_in[index].gl_Position;
    EmitVertex();
	lineColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
    gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].tangent, 0.0f) * MAGNITUDE;
    EmitVertex();
    EndPrimitive();

	//bitang	
	lineColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    gl_Position = gl_in[index].gl_Position;
    EmitVertex();
	lineColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    gl_Position = gl_in[index].gl_Position + vec4(gs_in[index].bitangent, 0.0f) * MAGNITUDE;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLines(0); // First vertex normal
    GenerateLines(1); // Second vertex normal
    GenerateLines(2); // Third vertex normal
} 