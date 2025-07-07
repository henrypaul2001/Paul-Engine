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
	float Resolution;
	float MaxDistance;
	float RayThickness;
	int NumBinarySearchSteps;
	float NormalAlignmentThreshold;
	float CameraNearPlane;
	int MaxRayMarchSteps;
} u_SSRData;

layout(binding = 0) uniform sampler2D Mat_gViewPosition;
layout(binding = 1) uniform sampler2D Mat_gWorldNormal;

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
		float depth = posSample.z;

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
	float rayJerk = 1.0;
	int maxSteps = 100;
	float depth = 0.0;
	vec4 projectedCoord = vec4(0.0);
	float acceleration = 1.0;

	// Sample position buffer along ray direction until the difference between the sample depth and ray depth is negative (collision with geometry)
	// If there is a collision, refine the ray with a binary search function (due to a large ray step, the ray may have stepped far behind geometry, so the ray needs to find its
	// way back to the geometry surface and find the actual point of collision)
	for (int i = 0; i < maxSteps; i++)
	{
		dir *= acceleration;
		acceleration *= rayJerk;
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

		if (posSample.z < -999.0)
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

vec3 PerspectiveCorrectLerp(vec3 A, vec3 B, float t_ndc)
{
	// avoid division by zero
	if (abs(A.z) < 0.001 || abs(B.z) < 0.001) return mix(A, B, t_ndc);

	float invA = 1.0 / A.z;
	float invB = 1.0 / B.z;
	float invInterp = mix(invA, invB, t_ndc);

	// avoid division by zero
	if (abs(invInterp) < 0.001) return mix(A, B, t_ndc);

	return mix(A * invA, B * invB, t_ndc) / invInterp;
}

// Returns the 0, 1 range UV coords and the refined hit distance
vec3 ScreenSpaceRayRefinementBinarySearch(vec2 pixelStart, vec2 pixelEnd, vec2 texelSize, vec3 viewSpaceRayStart, vec3 viewSpaceRayEnd, float hitDistance, float missDistance, int numSteps, out float out_DepthDifference)
{
	float depthDifference = 0.0;
	float refinedHitDistance = hitDistance;
	float refinedMissDistance = missDistance;
	vec2 uv = vec2(0.0);
	for (int i = 0; i < numSteps; i++)
	{
		float mid = (refinedHitDistance + refinedMissDistance) * 0.5;
		vec2 currentPixel = mix(pixelStart, pixelEnd, mid);

		// Convert to UV space coords
		uv = currentPixel * texelSize;

		vec3 positionSample = texture(Mat_gViewPosition, uv).xyz;
		if (positionSample.z < -999.0)
		{
			// Invalid sample (clear colour = vec4(-1000)
			continue;
		}

		// Perspective correct interpolation of view space line to find current ray depth
		vec3 currentViewSpaceRay = PerspectiveCorrectLerp(viewSpaceRayStart, viewSpaceRayEnd, mid);
		depthDifference = currentViewSpaceRay.z - positionSample.z;

		if (depthDifference < 0.0)
		{
			refinedHitDistance = mid;
		}
		else
		{
			refinedMissDistance = mid;
		}
	}

	out_DepthDifference = depthDifference;
	return vec3(uv, refinedHitDistance);
}

vec4 ScreenSpaceRayMarch(vec3 viewSpaceDirection, vec3 viewSpaceRayStart, out float out_DepthDifference)
{
	float maxRayDistance = u_SSRData.MaxDistance;
	float resolution = clamp(u_SSRData.Resolution, 0.0, 1.0);
	int numBinarySteps = u_SSRData.NumBinarySearchSteps;
	float thickness = u_SSRData.RayThickness;
	int maxSteps = u_SSRData.MaxRayMarchSteps;
	float normalAlignment = u_SSRData.NormalAlignmentThreshold;

	vec2 texSize = textureSize(Mat_gViewPosition, 0).xy;
	vec2 texelSize = 1.0 / texSize;

	vec3 viewSpaceRayEnd = viewSpaceRayStart + (viewSpaceDirection * maxRayDistance);

	// Rays going behind the camera can cause the projection to incorrectly flip the axis of the projected ray end point
	// Clip the ray to the camera near plane
	float paddedNearPlane = u_SSRData.CameraNearPlane * 2.0;
	if (viewSpaceRayEnd.z > -paddedNearPlane)
	{
		float safeZ = -paddedNearPlane;
		float t = min(maxRayDistance, (safeZ - viewSpaceRayStart.z) / viewSpaceDirection.z);
		if (t > 0.0)
		{
			viewSpaceRayEnd = viewSpaceRayStart + (viewSpaceDirection * t);
		}
	}

	// Project view space line into screen space
	vec4 projectedStart = u_CameraBuffer.Projection * vec4(viewSpaceRayStart, 1.0);
	projectedStart.xyz /= projectedStart.w;
	projectedStart.xyz = projectedStart.xyz * 0.5 + 0.5;

	vec4 projectedEnd = u_CameraBuffer.Projection * vec4(viewSpaceRayEnd, 1.0);
	projectedEnd.xyz /= projectedEnd.w;
	projectedEnd.xyz = projectedEnd.xyz * 0.5 + 0.5;

	// Convert screen space points into pixel space points
	vec2 pixelStart = projectedStart.xy * texSize;
	vec2 pixelEnd = projectedEnd.xy * texSize;

	// Calculate pixel space increment
	vec2 pixelDelta = pixelEnd - pixelStart;

	int deltaXLarger = (abs(pixelDelta.x) >= abs(pixelDelta.y)) ? 1 : 0;
	float scaledDelta = ((deltaXLarger == 1) ? abs(pixelDelta.x) : abs(pixelDelta.y)) * resolution;
	vec2 pixelIncrement = pixelDelta / scaledDelta;

	// Begin screen space ray march visiting each pixel on path
	// scaledDelta is the number of pixels along screen space line
	vec2 currentPixel = pixelStart;
	float distanceAlongRay = 0.0;
	float missDistance = 0.0;
	for (int i = 0; i < int(scaledDelta) && i < maxSteps; i++)
	{
		currentPixel += pixelIncrement;
		missDistance = distanceAlongRay;

		// Calculate current distance along ray
		if (deltaXLarger == 1)
		{
			distanceAlongRay = (currentPixel.x - pixelStart.x) / pixelDelta.x;
		}
		else { distanceAlongRay = (currentPixel.y - pixelStart.y) / pixelDelta.y; }

		// Convert to UV space coords
		vec2 uv = currentPixel * texelSize;

		if (uv.x < 0 || uv.y < 0 || uv.x > 1.0 || uv.y > 1.0)
		{
			// Attempting to sample outside of screen
			continue;
		}

		vec3 positionSample = texture(Mat_gViewPosition, uv).xyz;
		if (positionSample.z < -999.0)
		{
			// Invalid sample (clear colour = vec4(-1000)
			continue;
		}

		vec3 normalSample = mat3(u_CameraBuffer.View) * texture(Mat_gWorldNormal, uv).xyz;
		if (dot(normalSample, viewSpaceDirection) >= normalAlignment)
		{
			// Ray is moving away from surface, no collision
			continue;
		}

		// Perspective correct interpolation of view space line to find current ray depth
		vec3 currentViewSpaceRay = PerspectiveCorrectLerp(viewSpaceRayStart, viewSpaceRayEnd, distanceAlongRay);
		float depthDifference = currentViewSpaceRay.z - positionSample.z;

		if (abs(depthDifference) <= thickness)
		{
			// Hit somewhere between these two points
			vec3 refinedResult = ScreenSpaceRayRefinementBinarySearch(pixelStart, pixelEnd, texelSize, viewSpaceRayStart, viewSpaceRayEnd, distanceAlongRay, missDistance, numBinarySteps, depthDifference);
			uv = refinedResult.xy;
			distanceAlongRay = refinedResult.z;
			out_DepthDifference = depthDifference;
			return vec4(uv, distanceAlongRay, 1.0);
		}
	}

	out_DepthDifference = 0.0;
	return vec4(0.0);
}

float thickness = u_SSRData.RayThickness;
vec4 HiZ_ScreenSpaceRayMarch(vec3 viewSpaceDirection, vec3 viewSpaceRayStart, out float out_DepthDifference)
{
	float maxRayDistance = u_SSRData.MaxDistance;
	float resolution = clamp(u_SSRData.Resolution, 0.0, 1.0);
	int numBinarySteps = u_SSRData.NumBinarySearchSteps;
	//float thickness = u_SSRData.RayThickness;
	int maxSteps = u_SSRData.MaxRayMarchSteps;
	float normalAlignment = u_SSRData.NormalAlignmentThreshold;
	float rayOffset = 0.5;

	viewSpaceRayStart += viewSpaceDirection * rayOffset;
	vec3 currentViewSpaceRay = viewSpaceRayStart;
	vec3 viewSpaceRayEnd = viewSpaceRayStart + (viewSpaceDirection * maxRayDistance);

	// Rays going behind the camera can cause the projection to incorrectly flip the axis of the projected ray end point
	// Clip the ray to the camera near plane
	float paddedNearPlane = u_SSRData.CameraNearPlane * 2.0;
	if (viewSpaceRayEnd.z > -paddedNearPlane)
	{
		float safeZ = -paddedNearPlane;
		float t = min(maxRayDistance, (safeZ - viewSpaceRayStart.z) / viewSpaceDirection.z);
		if (t > 0.0)
		{
			viewSpaceRayEnd = viewSpaceRayStart + (viewSpaceDirection * t);
		}
	}

	// Project view space line into screen space
	vec4 projectedStart = u_CameraBuffer.Projection * vec4(viewSpaceRayStart, 1.0);
	projectedStart.xyz /= projectedStart.w;
	projectedStart.xyz = projectedStart.xyz * 0.5 + 0.5;

	vec4 projectedEnd = u_CameraBuffer.Projection * vec4(viewSpaceRayEnd, 1.0);
	projectedEnd.xyz /= projectedEnd.w;
	projectedEnd.xyz = projectedEnd.xyz * 0.5 + 0.5;

	const int targetMipBaseLevel = 2;
	int numMips = textureQueryLevels(Mat_gViewPosition);
	
	float distanceAlongRay = 0.0;
	for (int currentMip = min(targetMipBaseLevel, numMips); currentMip >= 0; currentMip--)
	{
		vec2 texSize = textureSize(Mat_gViewPosition, currentMip).xy;
		vec2 texelSize = 1.0 / texSize;

		// Convert screen space points into pixel space points
		vec2 pixelStart = projectedStart.xy * texSize;
		vec2 pixelEnd = projectedEnd.xy * texSize;

		// Calculate pixel space increment
		vec2 pixelDelta = pixelEnd - pixelStart;

		int deltaXLarger = (abs(pixelDelta.x) >= abs(pixelDelta.y)) ? 1 : 0;
		float scaledDelta = ((deltaXLarger == 1) ? abs(pixelDelta.x) : abs(pixelDelta.y)) * resolution;
		vec2 pixelIncrement = pixelDelta / scaledDelta;

		vec2 currentPixel = mix(pixelStart, pixelEnd, distanceAlongRay);
		
		// Begin screen space ray march visiting each pixel on path
		// scaledDelta is the number of pixels along screen space line
		float missDistance = 0.0;
		int hit = 0;
		for (int i = 0; i < int(scaledDelta) && i < maxSteps; i++)
		{
			currentPixel += pixelIncrement;
			missDistance = distanceAlongRay;

			vec2 pixelOffset = currentPixel - pixelStart;
			distanceAlongRay = dot(pixelOffset, pixelDelta) / dot(pixelDelta, pixelDelta);
			distanceAlongRay = clamp(distanceAlongRay, 0.0, 1.0);

			// Calculate current distance along ray
			if (deltaXLarger == 1)
			{
				distanceAlongRay = (currentPixel.x - pixelStart.x) / pixelDelta.x;
			}
			else { distanceAlongRay = (currentPixel.y - pixelStart.y) / pixelDelta.y; }

			// Convert to UV space coords
			vec2 uv = currentPixel * texelSize;

			if (uv.x < 0 || uv.y < 0 || uv.x > 1.0 || uv.y > 1.0)
			{
				// Attempting to sample outside of screen
				continue;
			}

			vec3 positionSample = texelFetch(Mat_gViewPosition, ivec2(currentPixel), currentMip).xyz;
			if (positionSample.z < -999.0)
			{
				// Invalid sample (clear colour = vec4(-1000)
				continue;
			}

			if (currentMip == 0)
			{
				vec3 normalSample = mat3(u_CameraBuffer.View) * texture(Mat_gWorldNormal, uv).xyz;
				if (dot(normalSample, viewSpaceDirection) >= normalAlignment)
				{
					// Ray is moving away from surface, no collision
					continue;
				}
			}

			// Perspective correct interpolation of view space line to find current ray depth
			currentViewSpaceRay = PerspectiveCorrectLerp(viewSpaceRayStart, viewSpaceRayEnd, distanceAlongRay);
			float depthDifference = currentViewSpaceRay.z - positionSample.z;

			if (abs(depthDifference) <= thickness)
			{
				// Check if this hit is the full resolution texture
				if (currentMip == 0)
				{
					// Hit somewhere between these two points
					vec3 refinedResult = ScreenSpaceRayRefinementBinarySearch(pixelStart, pixelEnd, texelSize, viewSpaceRayStart, viewSpaceRayEnd, distanceAlongRay, missDistance, numBinarySteps, depthDifference);
					uv = refinedResult.xy;
					distanceAlongRay = refinedResult.z;
					out_DepthDifference = depthDifference;
					return vec4(uv, distanceAlongRay, 1.0);
				}
				else
				{
					// Move back 1 pixel increment
					currentPixel = max(vec2(0.0), currentPixel - pixelIncrement);

					// Calculate current distance along ray
					if (deltaXLarger == 1)
					{
						distanceAlongRay = (currentPixel.x - pixelStart.x) / pixelDelta.x;
					}
					else { distanceAlongRay = (currentPixel.y - pixelStart.y) / pixelDelta.y; }

					// Go down a mip level for more detail
					hit = 1;


					break;
				}
			}
		}

		if (hit == 0)
		{
			break;
		}
	}

	out_DepthDifference = 0.0;
	return vec4(0.0);
}

void main()
{
	vec3 ViewSpaceFragPos = texture(Mat_gViewPosition, v_TexCoords).xyz;
	vec3 ViewSpaceNormal = mat3(u_CameraBuffer.View) * texture(Mat_gWorldNormal, v_TexCoords).xyz;

	vec3 UnitViewSpaceFragPos = normalize(ViewSpaceFragPos);
	vec3 UnitViewSpaceNormal = normalize(ViewSpaceNormal);

	vec3 reflected = normalize(reflect(UnitViewSpaceFragPos, UnitViewSpaceNormal));

	float maxThickness = thickness * 3.0;
	float angleFactor = max(dot(reflected, UnitViewSpaceFragPos), 0.0);
	thickness = mix(thickness, maxThickness, angleFactor);

	vec4 result = vec4(0.0);
	float depthDifference = 0.0;
	if (ViewSpaceFragPos.z >= -999.0)
	{
		// Valid ray start position
		result = HiZ_ScreenSpaceRayMarch(reflected, ViewSpaceFragPos, depthDifference);
	}

	vec2 coords = result.xy;
	float distanceAlongRay = result.z;
	float hit = result.a;

	vec2 centerDistance = smoothstep(0.1, 0.5, abs(vec2(0.5, 0.5) - coords));

	// TODO: add distance between ray start and camera scaling

	float screenEdgeFactor = clamp(1.0 - (centerDistance.x + centerDistance.y), 0.0, 1.0);
	float cameraDirectionFactor = (1.0 - max(dot(-UnitViewSpaceFragPos, reflected), 0.0));
	//float collisionAccuracyFactor = clamp(1.0 - smoothstep(0.0, u_SSRData.RayThickness, abs(depthDifference)), 0.0, 1.0);
	//float collisionAccuracyFactor = 1.0 - clamp(abs(depthDifference) / max(u_SSRData.RayThickness, 0.001), 0.0, 1.0);
	float collisionAccuracyFactor = 1.0;
	float distanceFromRayStartFactor = (1.0 - clamp(distanceAlongRay, 0.0, 1.0));

	float multiplier =
		hit * screenEdgeFactor
		* cameraDirectionFactor
		* collisionAccuracyFactor
		* distanceFromRayStartFactor;
	multiplier = clamp(multiplier, 0.0, 1.0);

	f_Colour = vec4(coords, multiplier, 0.0);
	f_MultiplierDebug = vec4(screenEdgeFactor, cameraDirectionFactor, collisionAccuracyFactor, distanceFromRayStartFactor);
}