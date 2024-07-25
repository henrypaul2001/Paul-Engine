#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];
uniform vec3 viewPos;

in VERTEX_VERT_OUTPUT {
	vec3 WorldPos;
	vec3 Normal;
	vec2 TexCoords;

    mat3 TBN;

	vec3 TangentFragPos;
} vertex_data;

out GEOMETRY_VERT_OUTPUT {
	vec3 WorldPos;
	vec3 Normal;
	vec2 TexCoords;

    mat3 TBN;

	vec3 TangentFragPos;
} out_vertex_data;

out GEOMETRY_VIEW_OUTPUT {
    flat vec3 TangentViewPos;
    flat vec3 ViewPos;
} view_data;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i)
        {
            out_vertex_data.WorldPos = vertex_data.WorldPos;
            out_vertex_data.Normal = vertex_data.Normal;
            out_vertex_data.TexCoords = vertex_data.TexCoords;
            out_vertex_data.TBN = vertex_data.TBN;
            out_vertex_data.TangentFragPos = vertex_data.TangentFragPos;

            view_data.TangentViewPos = vertex_data.TBN * viewPos;

            view_data.ViewPos = viewPos;

            vec4 FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
} 