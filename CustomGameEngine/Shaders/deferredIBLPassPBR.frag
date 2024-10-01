#version 430 core
layout (location = 0) out vec4 FragColour;
layout (location = 1) out vec4 BrightColour;

in vec2 TexCoords;

#define NR_LOCAL_REFLECTION_PROBES 32

struct DirLight {
    vec3 Direction;
    vec3 TangentDirection;
    
    mat4 LightSpaceMatrix;
    float LightDistance;

    bool CastShadows;
    bool Active;

    float MinShadowBias;
	float MaxShadowBias;

    vec2 shadowResolution;
    sampler2D ShadowMap;

    vec3 Colour;
    vec3 Ambient;
    vec3 Specular;
};
uniform DirLight dirLight;

uniform sampler2D lightingPass;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gArm;
uniform sampler2D gPBRFLAG;
uniform sampler2D SSAO;
uniform sampler2D ssrUVMap;

uniform sampler2D brdfLUT;

struct GlobalIBL {
    samplerCube irradianceMap;
    samplerCube prefilterMap;
};
uniform GlobalIBL globalIBL;
uniform bool useGlobalIBL;

struct AABB {
    float minX;
    float minY;
    float minZ;

    float maxX;
    float maxY;
    float maxZ;

    vec3 TransformMinToWorldSpace(vec3 origin) { return vec3(minX, minY, minZ) + origin; }
    vec3 TransformMaxToWorldSpace(vec3 origin) { return vec3(maxX, maxY, maxZ) + origin; }
};

struct LocalIBL {
    uint iblID;

    vec3 worldPos;
    float soiRadius; // Sphere of influence

    AABB geoApproximationAABB; // Bounding box representing approximation of surrounding geometry for parrallax correction
};
uniform LocalIBL localIBLProbes[NR_LOCAL_REFLECTION_PROBES];
uniform int activeLocalIBLProbes;
uniform samplerCubeArray localIBLIrradianceMapArray;
uniform samplerCubeArray localIBLPrefilterMapArray;

in vec3 ViewPos;

vec3 FragPos;
vec3 Albedo;
vec3 Normal;
float Metallic;
float Roughness;
float AO;
float AmbientOcclusion;
vec3 Lighting;

uniform float BloomThreshold;
uniform bool useSSAO;
uniform bool useSSR;

vec3 N;
vec3 V;
vec3 R;
vec3 RParallaxed;
vec3 F0;

const float PI = 3.14159265359;

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

bool IntersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 aabbMin, vec3 aabbMax, out vec3 intersectPoint) {
    vec3 invDir = 1.0 / rayDir; // Usefel to inverse the vector as it means we can now use multiplication ops instead of division

    vec3 t0 = (aabbMin - rayOrigin) * invDir; // Intersect near planes
    vec3 t1 = (aabbMax - rayOrigin) * invDir; // Intersect far planes
    
    vec3 tMin = min(t0, t1);
    vec3 tMax = max(t0, t1);

    float tNear = max(max(tMin.x, tMin.y), tMin.z);
    float tFar = min(min(tMax.x, tMax.y), tMax.z);

    if (tNear > tFar || tFar < 0.0) {
        return false; // No collision
    }

    intersectPoint = rayOrigin + tFar * rayDir;
    return true;
}

vec3 CalculateAmbienceFromIBL(uint iblIndex) {
        vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, Roughness);
        
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - Metallic;

        vec3 irradiance = texture(localIBLIrradianceMapArray, vec4(N, iblIndex)).rgb;
        vec3 diffuse = irradiance * Albedo;

        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColour = textureLod(localIBLPrefilterMapArray, vec4(RParallaxed, iblIndex), Roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), Roughness)).rg;
        vec3 specular = prefilteredColour * (F * brdf.x + brdf.y);

        return (kD * diffuse + specular) * AO;
}

vec3 CalculateAmbienceFromIBL(samplerCube prefilterMap, samplerCube irradianceMap) {
        vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, Roughness);
        
        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - Metallic;

        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse = irradiance * Albedo;

        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColour = textureLod(prefilterMap, RParallaxed, Roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), Roughness)).rg;
        vec3 specular = prefilteredColour * (F * brdf.x + brdf.y);

        return (kD * diffuse + specular) * AO;
}

vec3 ParallaxCorrectReflection(AABB aabb, vec3 original, vec3 correctionCenter) {
    vec3 rayOrigin = FragPos;
    vec3 rayDir = normalize(original);
    vec3 intersectPoint;

    if (IntersectAABB(rayOrigin, rayDir, aabb.TransformMinToWorldSpace(correctionCenter), aabb.TransformMaxToWorldSpace(correctionCenter), intersectPoint)) {
       return normalize(intersectPoint - correctionCenter);
    }

    return original;
}

vec3 CalculateAmbienceFromIBL(LocalIBL iblProbe) {
    // Run parallax correction
    RParallaxed = ParallaxCorrectReflection(iblProbe.geoApproximationAABB, R, iblProbe.worldPos);
    return CalculateAmbienceFromIBL(iblProbe.iblID);
}

void main() {
    if (texture(gPBRFLAG, TexCoords).r == 1.0) {
        // Retrieve data from gBuffer
        FragPos = texture(gPosition, TexCoords).rgb;
        Normal = texture(gNormal, TexCoords).rgb;
        Albedo = texture(gAlbedo, TexCoords).rgb;
        AO = texture(gArm, TexCoords).r;
        Roughness = texture(gArm, TexCoords).g;
        Metallic = texture(gArm, TexCoords).b;

        AmbientOcclusion = texture(SSAO, TexCoords).r;
        if (!useSSAO) {
            AmbientOcclusion = 1.0;
        }

        N = normalize(Normal);
        V = normalize(ViewPos - FragPos);
        R = reflect(-V, N);

        // calculate reflectance at normal incidence
	    // if dia-electric, use F0 of 0.04
	    // if metal, use albedo colour as F0
        F0 = vec3(0.04);
        F0 = mix(F0, Albedo, Metallic);

        vec3 Colour = texture(lightingPass, TexCoords).rgb; // sample lighting pass texture

        vec3 ambient;
        if (dirLight.Active) {
            ambient = (dirLight.Ambient * Albedo * AO) * AmbientOcclusion;
        }
        else {
            ambient = (vec3(0.01) * Albedo * AO) * AmbientOcclusion;
        }

	    float localIBLTotalContribution = 0.0;
        vec3 accumulatedAmbient = vec3(0.0);
        if (activeLocalIBLProbes > 0) {
            for (int i = 0; i < activeLocalIBLProbes && i < NR_LOCAL_REFLECTION_PROBES; i++) {
                float distanceToProbe = distance(FragPos, localIBLProbes[i].worldPos);

                float contribution = 1.0 - (distanceToProbe / localIBLProbes[i].soiRadius);

                //float contribution = 1.0 - (1.0 + distanceToProbe * distanceToProbe / (localIBLProbes[i].soiRadius * localIBLProbes[i].soiRadius));

                if (contribution > 0.0) {
                    vec3 probeAmbient = CalculateAmbienceFromIBL(localIBLProbes[i]);
                    accumulatedAmbient += probeAmbient * contribution;
                    localIBLTotalContribution += contribution;
                }
            }
        }

        if (localIBLTotalContribution > 0.0) {
            //ambient += (accumulatedAmbient / localIBLTotalContribution);
            ambient += accumulatedAmbient;
        }
    
        // Global IBL fallback
        if (useGlobalIBL && localIBLTotalContribution == 0.0) {
            RParallaxed = R;
            ambient = CalculateAmbienceFromIBL(globalIBL.prefilterMap, globalIBL.irradianceMap);
        }

        // SSR
        float ssrAlpha = 0.0;
        if (useSSR) {
            ssrAlpha = texture(ssrUVMap, TexCoords).b;
        }

        Colour += ambient * (1.0 - ssrAlpha);

        // Check whether result is higher than bloom threshold and output bloom colour accordingly
        float brightness = dot(Colour, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > BloomThreshold) {
            BrightColour = vec4(Colour, 1.0);
        }
        else {
            BrightColour = vec4(0.0, 0.0, 0.0, 1.0);
        }

        FragColour = vec4(Colour, 1.0);
    }
    else {
        vec3 Colour = texture(lightingPass, TexCoords).rgb;

        float brightness = dot(Colour, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > BloomThreshold) {
            BrightColour = vec4(Colour, 1.0);
        }
        else {
            BrightColour = vec4(0.0, 0.0, 0.0, 1.0);
        }

        FragColour = vec4(Colour, 1.0);
    }
}