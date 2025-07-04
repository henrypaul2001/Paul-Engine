// designed to be used after a deferred geometry pass, but doesn't contribute to the gBuffer itself
#context forward
#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoords;

layout(location = 1) out vec2 v_TexCoords;

void main()
{
	gl_Position = vec4(a_Position.xy, 0.0, 1.0);
	v_TexCoords = a_TexCoords;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 f_Colour;
layout(location = 1) out vec4 f_MultiplierDebug;

layout(location = 1) in vec2 v_TexCoords;

layout(std140, binding = 0) uniform Camera
{
	mat4 View;
	mat4 Projection;
	vec3 ViewPos;
	float Gamma;
	float Exposure;
} u_CameraBuffer;

layout(std140, binding = 3) uniform Mat_SSRData
{
	float RayJerk;
	float RayAcceleration;
	float RayStep;
	int MaxSteps;
	float MaxDistance;
	float RayThickness;
	int NumBinarySearchSteps;
	float NormalAlignmentThreshold;
} u_SSRData;

layout(binding = 0) uniform sampler2D Mat_gViewPosition;
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
		projectedCoord = u_CameraBuffer.Projection * vec4(ref_Hitcoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

		vec4 posSample = vec4(texture(Mat_gViewPosition, projectedCoord.xy).xyz, 1.0);
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

	projectedCoord = u_CameraBuffer.Projection * vec4(ref_Hitcoord, 1.0);
	projectedCoord.xy /= projectedCoord.w;
	projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

	return vec3(projectedCoord.xy, ref_dDepth);
}

// Issue:
// Smaller ray steps result in more accurate reflections, HOWEVER, a large number of steps is needed to reflect objects that are not close to the camera
// Larger ray steps result in more distant objects being reflected, HOWEVER, small objects close to the camera can be missed by the ray due to the ray stepping through the collision in one step
vec4 RayMarch(vec3 dir, inout vec3 ref_Hitcoord, out float out_dDepth, out int out_TotalSteps)
{
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

		projectedCoord = u_CameraBuffer.Projection * vec4(ref_Hitcoord, 1.0);
		projectedCoord.xyz /= projectedCoord.w;
		projectedCoord.xyz = projectedCoord.xyz * 0.5 + 0.5;

		if (projectedCoord.x < 0 || projectedCoord.y < 0 || projectedCoord.x > 1.0 || projectedCoord.y > 1.0)
		{
			// Attempting to sample outside of screen
			out_TotalSteps++;
			continue;
		}

		vec4 posSample = vec4(texture(Mat_gViewPosition, projectedCoord.xy).xyz, 1.0);
		depth = posSample.z;

		vec3 normalSample = mat3(u_CameraBuffer.View) * texture(Mat_gWorldNormal, projectedCoord.xy).xyz;
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
		if (out_dDepth <= u_SSRData.RayThickness)
		{
			return vec4(RayRefinementBinarySearch(dir, ref_Hitcoord, out_dDepth, u_SSRData.NumBinarySearchSteps), 1.0);
		}
	}

	return vec4(projectedCoord.xy, depth, 0.0);
}

vec4 HiZ_RayMarch(vec3 dir, inout vec3 ref_Hitcoord, out float out_dDepth, out int out_TotalSteps)
{
	ivec2 fullResSize = textureSize(Mat_gViewPosition, 0);

	vec3 rayViewSpace = ref_Hitcoord;
	vec3 normalizedViewSpaceDirection = normalize(dir);

	// View space to screen space
	vec4 projectedRayStart = u_CameraBuffer.Projection * vec4(rayViewSpace, 1.0);
	projectedRayStart.xyz /= projectedRayStart.w;
	projectedRayStart.xyz = projectedRayStart.xyz * 0.5 + 0.5;

	vec4 projectedRayEnd = u_CameraBuffer.Projection * vec4(rayViewSpace + normalizedViewSpaceDirection, 1.0);
	projectedRayEnd.xyz /= projectedRayEnd.w;
	projectedRayEnd.xyz = projectedRayEnd.xyz * 0.5 + 0.5;

	vec2 rayScreenSpace = projectedRayStart.xy;
	vec2 rayDirectionScreenSpace = normalize(projectedRayEnd.xy - projectedRayStart.xy);

	int currentMip = textureQueryLevels(Mat_gViewPosition) - 1;
	int maxSteps = 100;

	for (int numSteps = 0; numSteps < maxSteps && currentMip >= 0; numSteps++)
	{
		// Calculate screen space step size for current mip level
		ivec2 mipRes = textureSize(Mat_gViewPosition, currentMip);
		vec2 texelSize = 1.0 / vec2(mipRes);
		vec2 screenSpaceStep = rayDirectionScreenSpace * texelSize;

		if (rayScreenSpace.x < 0 || rayScreenSpace.y < 0 || rayScreenSpace.x > 1 || rayScreenSpace.y > 1)
		{
			// Ray is off screen
			break;
		}

		vec4 fullResViewSpaceSample = vec4(texelFetch(Mat_gViewPosition, ivec2(rayScreenSpace) * fullResSize, 0).xyz, 1.0);

		// Test HiZ buffer
		vec4 viewSpaceSample = vec4(texelFetch(Mat_gViewPosition, ivec2(rayScreenSpace) * mipRes, currentMip).xyz, 1.0);
		float viewSpaceMinDepth = viewSpaceSample.z;

		// Step view space ray to current screen space projected x, y coords to sample view space depth of the ray at this position
		rayViewSpace = rayViewSpace + dot(fullResViewSpaceSample.xyz - rayViewSpace, normalizedViewSpaceDirection) * normalizedViewSpaceDirection;
		float viewSpaceRayDepth = rayViewSpace.z;

		out_dDepth = viewSpaceRayDepth - viewSpaceMinDepth;

		if (out_dDepth <= u_SSRData.RayThickness)
		{
			// Possible collision in screen region
			currentMip--;

			if (currentMip < 0)
			{
				// Collision with full resolution buffer
				return vec4(RayRefinementBinarySearch(dir, rayViewSpace, out_dDepth, u_SSRData.NumBinarySearchSteps), 1.0);
			}
		}
		else
		{
			rayScreenSpace += screenSpaceStep;
		}

		out_TotalSteps++;
	}

	return vec4(rayScreenSpace.xy, 0.0, 0.0);
}

void main()
{
	mat4 View = u_CameraBuffer.View;
	mat4 Projection = u_CameraBuffer.Projection;

	vec3 ViewSpaceFragPos = texture(Mat_gViewPosition, v_TexCoords).xyz;
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