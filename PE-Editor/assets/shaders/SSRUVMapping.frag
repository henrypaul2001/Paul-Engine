#version 450 core

layout(location = 0) out vec4 f_Colour;
layout(location = 1) out vec4 f_MultiplierDebug;

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 3) uniform Mat_SSRData
{
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	float RayJerk;
	float RayAcceleration;
	float RayStep;
	int MaxSteps;
	float MaxDistance;
	float RayThickness;
	int NumBinarySearchSteps;
	float NormalAlignmentThreshold;
} u_SSRData;

layout(binding = 0) uniform sampler2D Mat_gWorldPosition;
layout(binding = 1) uniform sampler2D Mat_gWorldNormal;
layout(binding = 2) uniform sampler2D Mat_gMetadata;

vec3 RayRefinementBinarySearch(inout vec3 ref_Dir, inout vec3 ref_Hitcoord, inout float ref_dDepth, int binarySearchSteps)
{
	float depth = 0.0;
	vec4 projectedCoord = vec4(0.0);

	// Step the ray either forwards or backwards to find the actual collision point
	// Each step shortens the distance of the next step. More steps = higher precision
	for (int i = 0; i < binarySearchSteps; i++)
	{
		projectedCoord = u_SSRData.ProjectionMatrix * vec4(ref_Hitcoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

		vec4 posSample = u_SSRData.ViewMatrix * vec4(texture(Mat_gWorldPosition, projectedCoord.xy).xyz, 1.0);
		depth = posSample.z;

		ref_dDepth = ref_Hitcoord.z - depth;

		ref_Dir *= 0.5;
		if (ref_dDepth > 0.0)
		{
			ref_Hitcoord += ref_Dir;
		}
		else
		{
			ref_Hitcoord -= ref_Dir;
		}
	}

	projectedCoord = u_SSRData.ProjectionMatrix * vec4(ref_Hitcoord, 1.0);
	projectedCoord.xy /= projectedCoord.w;
	projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

	return vec3(projectedCoord.xy, ref_dDepth);
}

vec4 RayMarch(vec3 dir, inout vec3 ref_Hitcoord, out float out_dDepth, out int out_TotalSteps)
{
	dir *= u_SSRData.RayAcceleration;

	float depth = 0.0;
	vec4 projectedCoord = vec4(0.0);
	float acceleration = u_SSRData.RayAcceleration;

	// Sample position buffer along ray direction until the difference between the sample depth and ray depth is negative (collision with geometry)
	// If there is a collision, refine the ray with a binary search function (due to a large ray step, the ray may have stepped far behind geometry, so the ray needs to find its
	// way back to the geometry surface and find the actual point of collision)
	for (int i = 0; i < u_SSRData.MaxSteps; i++)
	{
		dir *= acceleration;
		acceleration *= u_SSRData.RayJerk;
		ref_Hitcoord += dir;

		projectedCoord = u_SSRData.ProjectionMatrix * vec4(ref_Hitcoord, 1.0);
		projectedCoord.xyz /= projectedCoord.w;
		projectedCoord.xyz = projectedCoord.xyz * 0.5 + 0.5;

		if (projectedCoord.x < 0 || projectedCoord.y < 0 || projectedCoord.x > 1.0 || projectedCoord.y > 1.0)
		{
			// Attempting to sample outside of screen
			out_TotalSteps++;
			continue;
		}

		vec4 posSample = u_SSRData.ViewMatrix * vec4(texture(Mat_gWorldPosition, projectedCoord.xy).xyz, 1.0);
		depth = posSample.z;

		vec3 normalSample = mat3(u_SSRData.ViewMatrix) * texture(Mat_gWorldNormal, projectedCoord.xy).xyz;
		if (dot(normalSample, dir) >= u_SSRData.NormalAlignmentThreshold)
		{
			// Ray is moving away from surface, no collision
			out_TotalSteps++;
			continue;
		}

		float lightingModel = texture(Mat_gMetadata, projectedCoord.xy).y;
		if (lightingModel < -0.1)
		{
			// Invalid hit, nothing is written at this position
			out_TotalSteps++;
			continue;
		}

		if (depth > 1000.0)
		{
			out_TotalSteps++;
			continue;
		}

		out_dDepth = ref_Hitcoord.z - depth;

		out_TotalSteps++;
		if (out_dDepth <= 0.0)
		{
			return vec4(RayRefinementBinarySearch(dir, ref_Hitcoord, out_dDepth, u_SSRData.NumBinarySearchSteps), 1.0);
		}
	}

	return vec4(projectedCoord.xy, depth, 0.0);
}

void main()
{
	mat4 View = u_SSRData.ViewMatrix;
	mat4 Projection = u_SSRData.ProjectionMatrix;

	vec3 ViewSpaceFragPos = vec3(View * vec4(texture(Mat_gWorldPosition, v_TexCoords).xyz, 1.0));
	vec3 ViewSpaceNormal = mat3(View) * texture(Mat_gWorldNormal, v_TexCoords).xyz;

	vec3 UnitViewSpaceFragPos = normalize(ViewSpaceFragPos);
	vec3 UnitViewSpaceNormal = normalize(ViewSpaceNormal);

	vec3 reflected = normalize(reflect(UnitViewSpaceFragPos, UnitViewSpaceNormal));

	vec3 hitPos = ViewSpaceFragPos;
	float dDepth;
	int steps = 0;

	vec4 coords = vec4(0.0);
	float lightingModel = texture(Mat_gMetadata, v_TexCoords.xy).y;
	if (lightingModel > 0.0)
	{
		// Valid ray start position
		coords = RayMarch(reflected * u_SSRData.RayStep, hitPos, dDepth, steps);
	}

	vec2 centerDistance = smoothstep(0.1, 0.5, abs(vec2(0.5, 0.5) - v_TexCoords));

	float screenEdgeFactor = clamp(1.0 - (centerDistance.x + centerDistance.y), 0.0, 1.0);
	float cameraDirectionFactor = (1 - max(dot(-UnitViewSpaceFragPos, reflected), 0.0));
	float collisionAccuracyFactor = clamp(1 - smoothstep(0.0, u_SSRData.RayThickness, abs(dDepth)), 0.0, 1.0);
	//float collisionAccuracyFactor = 1.0 - clamp(abs(dDepth) / u_SSRData.RayThickness, 0.0, 1.0);
	float distanceFromRayStartFactor = (1 - clamp(length(hitPos - ViewSpaceFragPos) / u_SSRData.MaxDistance, 0.0, 1.0));
	float hit = coords.a; // hit == 1, no hit == 0

	float multiplier =
		hit * screenEdgeFactor
		* cameraDirectionFactor
		* collisionAccuracyFactor
		* distanceFromRayStartFactor;

	multiplier = clamp(multiplier, 0.0, 1.0);

	f_Colour = vec4(coords.xy, vec2(multiplier, steps));
	f_MultiplierDebug = vec4(screenEdgeFactor, cameraDirectionFactor, collisionAccuracyFactor, distanceFromRayStartFactor);
}