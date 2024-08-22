#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 24) out;

uniform float MinX, MinY, MinZ, MaxX, MaxY, MaxZ;

// uniform block
layout (std140) uniform Common
{
    mat4 projection;
    mat4 view;
    vec3 viewPos;
};

void EmitLine(vec3 start, vec3 end) {
	gl_Position = projection * view * vec4(start, 1.0);
	EmitVertex();

	gl_Position = projection * view * vec4(end, 1.0);
	EmitVertex();

	EndPrimitive();
}

void main() {
	vec3 minPoint = gl_in[0].gl_Position.xyz + vec3(MinX, MinY, MinZ);
	vec3 maxPoint = gl_in[0].gl_Position.xyz + vec3(MaxX, MaxY, MaxZ);

	// Emit 12 lines for each edge of AABB
	EmitLine(minPoint, vec3(minPoint.x, minPoint.y, maxPoint.z));
	EmitLine(minPoint, vec3(minPoint.x, maxPoint.y, minPoint.z));
	EmitLine(minPoint, vec3(maxPoint.x, minPoint.y, minPoint.z));

	EmitLine(maxPoint, vec3(maxPoint.x, maxPoint.y, minPoint.z));
    EmitLine(maxPoint, vec3(maxPoint.x, minPoint.y, maxPoint.z));
    EmitLine(maxPoint, vec3(minPoint.x, maxPoint.y, maxPoint.z));

	EmitLine(vec3(minPoint.x, minPoint.y, maxPoint.z), vec3(minPoint.x, maxPoint.y, maxPoint.z));
    EmitLine(vec3(minPoint.x, minPoint.y, maxPoint.z), vec3(maxPoint.x, minPoint.y, maxPoint.z));

    EmitLine(vec3(minPoint.x, maxPoint.y, minPoint.z), vec3(minPoint.x, maxPoint.y, maxPoint.z));
    EmitLine(vec3(minPoint.x, maxPoint.y, minPoint.z), vec3(maxPoint.x, maxPoint.y, minPoint.z));

    EmitLine(vec3(maxPoint.x, minPoint.y, minPoint.z), vec3(maxPoint.x, minPoint.y, maxPoint.z));
    EmitLine(vec3(maxPoint.x, minPoint.y, minPoint.z), vec3(maxPoint.x, maxPoint.y, minPoint.z));
}