#version 430 core
layout (location = 0) out vec4 FragColour;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

in mat4 Projection;
in mat4 View;
in vec2 TexCoords;

// Parameters
uniform float rayStep = 0.3;
uniform float minRayStep = 0.3;
uniform float maxSteps = 100;
uniform float maxDistance = 15.0;
uniform float rayThickness = 0.3;
uniform int numBinarySearchSteps = 50;

vec3 RayRefinementBinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth) {
	float depth;
	vec4 projectedCoord;

	for (int i = 0; i < numBinarySearchSteps; i++) {
		projectedCoord = Projection * vec4(hitCoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

		vec4 posSample = texture(gPosition, projectedCoord.xy);
		depth = vec3(View * vec4(posSample.xyz, 1.0)).z;

		dDepth = hitCoord.z - depth;

		dir *= 0.5;
		if (dDepth > 0.0) {
			hitCoord += dir;
		}
		else {
			hitCoord -= dir;
		}
	}

	projectedCoord = Projection * vec4(hitCoord, 1.0);
	projectedCoord.xy /= projectedCoord.w;
	projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

	return vec3(projectedCoord.xy, depth);
}

vec4 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth) {
	dir *= rayStep;

	float depth;
	int steps;
	vec4 projectedCoord;

	for (int i = 0; i < maxSteps; i++) {
		hitCoord += dir;

		projectedCoord = Projection * vec4(hitCoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

		vec4 posSample = texture(gPosition, projectedCoord.xy);
		depth = vec3(View * vec4(posSample.xyz, 1.0)).z;

		if (depth > 1000.0 || posSample.a == 0.0) {
			continue;
		}

		dDepth = hitCoord.z - depth;

		if ((dir.z - dDepth) < rayThickness) {
			if (dDepth <= 0.0) {
				return vec4(RayRefinementBinarySearch(dir, hitCoord, dDepth), 1.0);
			}
		}
		steps++;
	}

	return vec4(projectedCoord.xy, depth, 0.0);
}

void main() {
	vec3 viewSpaceFragPos = vec3(View * vec4(texture(gPosition, TexCoords).xyz, 1.0));
	vec3 viewSpaceNormal = mat3(View) * texture(gNormal, TexCoords).rgb;

	vec3 reflected = normalize(reflect(normalize(viewSpaceFragPos), normalize(viewSpaceNormal)));
	vec3 hitPos = viewSpaceFragPos;
	float dDepth;

	vec4 coords = RayMarch(reflected * max(minRayStep, -viewSpaceFragPos.z), hitPos, dDepth);

	int hit = 1;
	if (coords.a == 0.0) {
		hit = 0;
	}

	vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));

	float screenEdgeFactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);

	float multiplier = screenEdgeFactor
					* (1 - max(dot(normalize(-viewSpaceFragPos), reflected), 0.0))				// Fade if pointing towards camera
					* (1 - clamp(dDepth / rayThickness, 0.0, 1.0))								// Fade reflection the further away from intersect point
					* (1 - clamp(length(hitPos - viewSpaceFragPos) / maxDistance, 0.0, 1.0))    // Fade based on distance to initial ray start point
					* hit;
	multiplier = clamp(multiplier, 0.0, 1.0);

	FragColour = vec4(coords.xy, vec2(multiplier));
}