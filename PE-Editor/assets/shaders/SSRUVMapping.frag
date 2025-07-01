#version 450 core

layout(location = 0) out vec4 f_Colour;
layout(location = 1) out vec4 f_MultiplierDebug;

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 3) uniform Mat_SSRData
{
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	float RayAcceleration;
	float RayStep;
	int MaxSteps;
	float MaxDistance;
	float RayThickness;
	int NumBinarySearchSteps;
} u_SSRData;

layout(binding = 0) uniform sampler2D Mat_gWorldPosition;
layout(binding = 1) uniform sampler2D Mat_gWorldNormal;
layout(binding = 2) uniform sampler2D Mat_gSpecular;
layout(binding = 3) uniform sampler2D Mat_gARM;
layout(binding = 4) uniform sampler2D Mat_gMetadata;

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

	// Sample position buffer along ray direction until the difference between the sample depth and ray depth is negative (collision with geometry)
	// If there is a collision, refine the ray with a binary search function (due to a large ray step, the ray may have stepped far behind geometry, so the ray needs to find its
	// way back to the geometry surface and find the actual point of collision)
	for (int i = 0; i < u_SSRData.MaxSteps; i++)
	{
		ref_Hitcoord += dir;

		projectedCoord = u_SSRData.ProjectionMatrix * vec4(ref_Hitcoord, 1.0);
		projectedCoord.xyz /= projectedCoord.w;
		projectedCoord.xyz = projectedCoord.xyz * 0.5 + 0.5;

		vec4 posSample = u_SSRData.ViewMatrix * vec4(texture(Mat_gWorldPosition, projectedCoord.xy).xyz, 1.0);
		depth = posSample.z;

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

	float roughness = 0.0;
	int LightingModelIndex = int(texture(Mat_gMetadata, v_TexCoords).g); // 0 = blinn-phong, 1 = pbr
	if (LightingModelIndex == 0)
	{
		float SpecularExponent = texture(Mat_gSpecular, v_TexCoords).a;
		roughness = 1.0 - (SpecularExponent / 256.0);
	}
	else if (LightingModelIndex == 1)
	{
		vec3 armSample = texture(Mat_gARM, v_TexCoords).rgb;
		roughness = armSample.g;
	}

	vec3 UnitViewSpaceFragPos = normalize(ViewSpaceFragPos);
	vec3 UnitViewSpaceNormal = normalize(ViewSpaceNormal);

	vec3 reflected = normalize(reflect(UnitViewSpaceFragPos, UnitViewSpaceNormal));

	vec3 hitPos = ViewSpaceFragPos;
	float dDepth;
	int steps = 0;

	vec4 coords = RayMarch(reflected * u_SSRData.RayStep, hitPos, dDepth, steps);
	vec2 dCoords = smoothstep(0.2, 0.6, abs(vec2(0.5, 0.5) - coords.xy));

	float screenEdgeFactor = clamp(1.0 - (dCoords.x + dCoords.y), 0.0, 1.0);
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