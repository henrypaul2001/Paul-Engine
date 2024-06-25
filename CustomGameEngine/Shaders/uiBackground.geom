#version 330 core
layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform vec4 LeftRightUpDownExtents;

out vec2 TexCoords;

void main() {
	vec4 p = gl_in[0].gl_Position;

	float left = -LeftRightUpDownExtents.x;
	float right = LeftRightUpDownExtents.y;
	float up = LeftRightUpDownExtents.z;
	float down = LeftRightUpDownExtents.w;

	// bottom-left
	vec2 a = p.xy + vec2(left, down);
	gl_Position = vec4(a, p.zw);
	TexCoords = vec2(0.0, 0.0);
	EmitVertex();

	// top-left
	vec2 b = p.xy + vec2(left, up);
	gl_Position = vec4(b, p.zw);
	TexCoords = vec2(0.0, 1.0);
	EmitVertex();

	// bottom-right
	vec2 d = p.xy + vec2(right, down);
	gl_Position = vec4(d, p.zw);
	TexCoords = vec2(1.0, 0.0);
	EmitVertex();

	// top-right
	vec2 c = p.xy + vec2(right, up);
	gl_Position = vec4(c, p.zw);
	TexCoords = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}