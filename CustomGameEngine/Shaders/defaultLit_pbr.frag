#version 330 core
out vec4 FragColor;

in VIEW_DATA {
    flat vec3 TangentViewPos;
    flat vec3 ViewPos;
} view_data;

in VERTEX_DATA {
	vec3 WorldPos;
	vec3 Normal;
	vec2 TexCoords;

    mat3 TBN;

	vec3 TangentFragPos;
} vertex_data;

void main() {
	FragColor = vec4(1.0);
}