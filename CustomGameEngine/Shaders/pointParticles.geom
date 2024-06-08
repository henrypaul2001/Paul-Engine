#version 330 core
layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

in vec2 Scale[];
in vec4 Colour[];

out vec2 TexCoords;
out vec4 ParticleColour;

void main() {
	vec4 p = gl_in[0].gl_Position;

	// bottom-left
	vec2 a = p.xy + vec2(-0.5 * Scale[0].x, -0.5 * Scale[0].y);
	gl_Position = vec4(a, p.zw);
	TexCoords = vec2(0.0, 0.0);
	ParticleColour = Colour[0];
	EmitVertex();

	// top-left
	vec2 b = p.xy + vec2(-0.5 * Scale[0].x, 0.5 * Scale[0].y);
	gl_Position = vec4(b, p.zw);
	TexCoords = vec2(0.0, 1.0);
	ParticleColour = Colour[0];
	EmitVertex();

	// bottom-right
	vec2 d = p.xy + vec2(0.5 * Scale[0].x, -0.5 * Scale[0].y);
	gl_Position = vec4(d, p.zw);
	TexCoords = vec2(1.0, 0.0);
	ParticleColour = Colour[0];
	EmitVertex();

	// top-right
	vec2 c = p.xy + vec2(0.5 * Scale[0].x, 0.5 * Scale[0].y);
	gl_Position = vec4(c, p.zw);
	TexCoords = vec2(1.0, 1.0);
	ParticleColour = Colour[0];
	EmitVertex();

	EndPrimitive();
}