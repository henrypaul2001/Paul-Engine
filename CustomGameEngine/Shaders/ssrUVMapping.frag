#version 430 core
layout (location = 0) out vec4 FragColour;

uniform sampler2D gViewSpacePos;
uniform sampler2D gNormal;

in mat4 Projection;
in mat4 View;
in vec2 TexCoords;

// Parameters
uniform float rayStep = 1.0;
uniform float minRayStep = 0.3;
uniform float maxSteps = 30;
uniform float maxDistance = 50.0;
uniform float rayThickness = 0.3;
uniform int numBinarySearchSteps = 50;

vec3 RayRefinementBinarySearch(inout vec3 dir, inout vec3 hitCoord, inout float dDepth) {
	float depth;
	vec4 projectedCoord;

	for (int i = 0; i < numBinarySearchSteps; i++) {
		projectedCoord = Projection * vec4(hitCoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

		vec4 posSample = texture(gViewSpacePos, projectedCoord.xy);
		depth = vec4(posSample.xyz, 1.0).z;

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

vec4 RayMarch(vec3 dir, inout vec3 hitCoord, out float dDepth, out int totalSteps) {
	dir *= rayStep;

	float depth;
	vec4 projectedCoord;

	for (int i = 0; i < maxSteps; i++) {
		hitCoord += dir;

		projectedCoord = Projection * vec4(hitCoord, 1.0);
		projectedCoord.xyz /= projectedCoord.w;
		projectedCoord.xyz = projectedCoord.xyz * 0.5 + 0.5;

		vec4 posSample = texture(gViewSpacePos, projectedCoord.xy);
		depth = vec4(posSample.xyz, 1.0).z;

		if (depth > 1000.0 || posSample.a == 0.0) {
			totalSteps++;
			continue;
		}

		dDepth = hitCoord.z - depth;

		if (abs(dDepth) < rayThickness) {
			if (dDepth <= 0.0) {
				return vec4(RayRefinementBinarySearch(dir, hitCoord, dDepth), 1.0);
			}
		}
		totalSteps++;
	}

	return vec4(projectedCoord.xy, depth, 0.0);
}

void main() {
	vec3 viewSpaceFragPos = texture(gViewSpacePos, TexCoords).xyz;
	vec3 viewSpaceNormal = mat3(View) * texture(gNormal, TexCoords).rgb;

	vec3 reflected = normalize(reflect(normalize(viewSpaceFragPos), normalize(viewSpaceNormal)));
	vec3 hitPos = viewSpaceFragPos;
	float dDepth;
	int steps = 0;

	vec4 coords = RayMarch(reflected * minRayStep, hitPos, dDepth, steps);
	vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));

	float screenEdgeFactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);
	float cameraDirectionFactor = (1 - max(dot(normalize(-viewSpaceFragPos), reflected), 0.0));
	float collisionAccuracyFactor = clamp(1 - smoothstep(0.0, rayThickness, abs(dDepth)), 0.0, 1.0);
	float distanceFromRayStartFactor = (1 - clamp(length(hitPos - viewSpaceFragPos) / maxDistance, 0.0, 1.0));

	float multiplier = coords.a * screenEdgeFactor  // coords.a == 1 if hit, 0 if no hit
					* cameraDirectionFactor			// Fade if pointing towards camera
					* collisionAccuracyFactor		// Fade reflection the further away from intersect point
					* distanceFromRayStartFactor;   // Fade based on distance to initial ray start point
	multiplier = clamp(multiplier, 0.0, 1.0);

	FragColour = vec4(coords.xy, vec2(multiplier, steps));
}