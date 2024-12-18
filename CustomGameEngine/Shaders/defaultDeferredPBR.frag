#version 400 core
layout (location = 0) out vec4 FragColour;
layout (location = 1) out vec4 BrightColour;

in vec2 TexCoords;

#define NR_LOCAL_REFLECTION_PROBES 32
#define NR_REAL_TIME_LIGHTS 8
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
struct Light {
    vec3 Position; // universal
    vec3 TangentPosition;

    vec3 Colour; // universal
    vec3 Ambient; // universal
    vec3 Specular; // universal

    float Linear; // universal
    float Quadratic; // universal
    float Constant; // universal

    mat4 LightSpaceMatrix;
    
    bool CastShadows;
    bool Active;

    float MinShadowBias;
	float MaxShadowBias;
    float ShadowFarPlane; // point light specific

    vec2 spotShadowAtlasTexOffset;
    vec2 shadowResolution;

    bool SpotLight;
    vec3 Direction; // spotlight specific
    vec3 TangentDirection; // spotlight specific

    float Cutoff; // spotlight specific
    float OuterCutoff; // spotlight specific
};

uniform DirLight dirLight;
uniform Light lights[NR_REAL_TIME_LIGHTS];
uniform int activeLights;
uniform sampler2D spotlightShadowAtlas;
uniform samplerCubeArray pointLightShadowArray;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gArm;
uniform sampler2D gPBRFLAG;
uniform sampler2D SSAO;

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

uniform sampler2D nonPBRResult;
uniform sampler2D nonPBRBrightResult;

in vec3 ViewPos;

vec3 FragPos;
vec3 Albedo;
vec3 Normal;
float Metallic;
float Roughness;
float AO;
vec3 Lighting;
vec3 ViewDir;
float AmbientOcclusion;

uniform bool useSSAO;

uniform float BloomThreshold;

vec3 N;
vec3 V;
vec3 R;
vec3 RParallaxed;
vec3 F0;

const float PI = 3.14159265359;

float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap, vec3 lightPos, float minBias, float maxBias, vec2 shadowRes, vec2 atlasOffset) {
    // perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0, 1]
    projCoords.xyz = projCoords.xyz * 0.5 + 0.5;

    if (projCoords.z > 1.0) {
        return 0.0;
    }
    
    // offset texcoords for texture atlas
    vec2 startCoords = atlasOffset / textureSize(shadowMap, 0);
    vec2 slotRes = shadowRes / textureSize(shadowMap, 0);

    projCoords.xy = projCoords.xy * slotRes + startCoords;

    // get closes depth value from lights perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // get depth of current fragment
    float currentDepth = projCoords.z;

    // check if current frag pos is in shadow
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(maxBias * (1.0 - dot(Normal, lightDir)), minBias);

    // pcf soft shadows (simple solution but more advanced solutions out there)
    float shadow = 0.0;
    vec2 texelSize = slotRes / shadowRes;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);
float CubeShadowCalculation(int lightIndex) {
    vec3 fragToLight = FragPos - lights[lightIndex].Position;

    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = max(lights[lightIndex].MaxShadowBias * (1.0 - dot(Normal, fragToLight)), lights[lightIndex].MinShadowBias);
    //float bias = 0.15;
    int samples = 20;
    
    float viewDistance = length(ViewPos - FragPos);
    float diskRadius = (1.0 + (viewDistance / lights[lightIndex].ShadowFarPlane)) / 25.0;

    for (int i = 0; i < samples; i++) {
        float closestDepth = texture(pointLightShadowArray, vec4(fragToLight + gridSamplingDisk[i] * diskRadius, lightIndex)).r;
        closestDepth *= lights[lightIndex].ShadowFarPlane;

        if (currentDepth - bias > closestDepth) {
            shadow += 1.0;
        }
    }

    shadow /= float(samples);

    return shadow;
}

float DistrubitionGGX(vec3 H) {
    float a = Roughness * Roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV) {
	float r = (Roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 L) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV);
    float ggx1 = GeometrySchlickGGX(NdotL);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 PerLightReflectance_SpotLight(int lightIndex) {
    // Radiance
    vec3 L = normalize(lights[lightIndex].Position - FragPos);
    vec3 H = normalize(V + L);

    float dist = length(lights[lightIndex].Position - FragPos);
    float attenuation = 1.0 / (lights[lightIndex].Constant + lights[lightIndex].Linear * dist + lights[lightIndex].Quadratic * (dist * dist));
    vec3 radiance = lights[lightIndex].Colour * attenuation;

    // Spot light
    float theta = dot(L, normalize(-lights[lightIndex].Direction));
    float epsilon = lights[lightIndex].Cutoff - lights[lightIndex].OuterCutoff;
    float intensity = clamp((theta - lights[lightIndex].OuterCutoff) / epsilon, 0.0, 1.0);

    // Cook-Torrance BRDF
    float NDF = DistrubitionGGX(H);
    float G = GeometrySmith(L);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    // kS is equal to fresnel
    vec3 kS = F;
    //kS *= intensity;

    // For energy conservation, the diffuse and specular light can't be above 1.0 (unless the surface emits light)
	// To preserve this relationship the diffuse component (kD) should equal 1.0 - kS
	vec3 kD = vec3(1.0) - kS;

    // Multiply kD by inverse metalness such that only non-metals have diffuse lighting , or linear blend if partly metal
	kD *= 1.0 - Metallic;

    // Scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);

    kD *= intensity;
    specular *= intensity;
    radiance *= intensity;

    if (lights[lightIndex].CastShadows) {
        // Calculate shadow
        float shadow = ShadowCalculation(lights[lightIndex].LightSpaceMatrix * vec4(FragPos, 1.0), spotlightShadowAtlas, lights[lightIndex].Position, lights[lightIndex].MinShadowBias, lights[lightIndex].MaxShadowBias, lights[lightIndex].shadowResolution, lights[lightIndex].spotShadowAtlasTexOffset);
        kD *= (1.0 - shadow);
        specular *= (1.0 - shadow);
        radiance *= (1.0 - shadow);
        NdotL *= (1.0 - shadow);
    }

    // Add to outgoing radiance Lo
    vec3 Lo = (kD * Albedo / PI + specular) * radiance * NdotL;
    return Lo;
    
    return vec3(0.0);
}

vec3 PerLightReflectance_DirLight() {
    // Radiance
    vec3 L = normalize(-dirLight.Direction);
    vec3 H = normalize(V + L);

    vec3 radiance = dirLight.Colour;

    // Cook-Torrance BRDF
    float NDF = DistrubitionGGX(H);
    float G = GeometrySmith(L);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    // kS is equal to fresnel
    vec3 kS = F;

    // For energy conservation, the diffuse and specular light can't be above 1.0 (unless the surface emits light)
	// To preserve this relationship the diffuse component (kD) should equal 1.0 - kS
	vec3 kD = vec3(1.0) - kS;

    // Multiply kD by inverse metalness such that only non-metals have diffuse lighting , or linear blend if partly metal
	kD *= 1.0 - Metallic;

    // Scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);

    if (dirLight.CastShadows) {
        // Calculate shadow
        float shadow = ShadowCalculation(dirLight.LightSpaceMatrix * vec4(FragPos, 1.0), dirLight.ShadowMap, -dirLight.Direction * dirLight.LightDistance, dirLight.MinShadowBias, dirLight.MaxShadowBias, dirLight.shadowResolution, vec2(0.0, 0.0));
        kD *= (1.0 - shadow);
        specular *= (1.0 - shadow);
        radiance *= (1.0 - shadow);
    }

    vec3 Lo = (kD * Albedo / PI + specular) * radiance * NdotL;

    // Add to outgoing radiance Lo
    return Lo;
}

vec3 PerLightReflectance_PointLight(int lightIndex) {
    // Radiance
    vec3 L = normalize(lights[lightIndex].Position - FragPos);
    vec3 H = normalize(V + L);

    float dist = length(lights[lightIndex].Position - FragPos);
    float attenuation = 1.0 / (dist * dist);
    //float attenuation = 1.0 / (lights[lightIndex].Constant + lights[lightIndex].Linear * dist + lights[lightIndex].Quadratic * (dist * dist));
    vec3 radiance = lights[lightIndex].Colour * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistrubitionGGX(H);
    float G = GeometrySmith(L);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    // kS is equal to fresnel
    vec3 kS = F;

    // For energy conservation, the diffuse and specular light can't be above 1.0 (unless the surface emits light)
	// To preserve this relationship the diffuse component (kD) should equal 1.0 - kS
	vec3 kD = vec3(1.0) - kS;

    // Multiply kD by inverse metalness such that only non-metals have diffuse lighting , or linear blend if partly metal
	kD *= 1.0 - Metallic;

    // Scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);

    if (lights[lightIndex].CastShadows) {
        // Calculate shadow
        float shadow = CubeShadowCalculation(lightIndex);
        kD *= (1.0 - shadow);
        specular *= (1.0 - shadow);
        radiance *= (1.0 - shadow);
        NdotL *= (1.0 - shadow);
    }

    // Add to outgoing radiance Lo
    return (kD * Albedo / PI + specular) * radiance * NdotL;
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
        // Run lighting calculations for pbr pixel
        // Retrieve data from gBuffer
        FragPos = texture(gPosition, TexCoords).rgb;
        Normal = texture(gNormal, TexCoords).rgb;
        Albedo = texture(gAlbedo, TexCoords).rgb;
        AO = texture(gArm, TexCoords).r;
        Roughness = texture(gArm, TexCoords).g;
        Metallic = texture(gArm, TexCoords).b;
        AmbientOcclusion = texture(SSAO, TexCoords).r;
        ViewDir = normalize(ViewPos - FragPos);

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

        // per-light reflectance equation
        vec3 Lo = vec3(0.0);

        if (dirLight.Active) {
            Lo += PerLightReflectance_DirLight();
        }
        for (int i = 0; i < activeLights && i < NR_REAL_TIME_LIGHTS; i++) {
            if (lights[i].Active) {
                if (lights[i].SpotLight) {
                    Lo += PerLightReflectance_SpotLight(i);
                }
                else {
                    Lo += PerLightReflectance_PointLight(i);
                }
            }
        }

        // Ambient lighting
        vec3 ambient = vec3(0.0);
        // If using IBL, IBL pass will handle this
        if (activeLocalIBLProbes <= 0 || useGlobalIBL) {
            if (dirLight.Active) {
                ambient = (dirLight.Ambient * Albedo * AO) * AmbientOcclusion;
            }
            else {
                ambient = (vec3(0.01) * Albedo * AO) * AmbientOcclusion;
            }
        }

        vec3 Colour = ambient + Lo;

        // Check whether result is higher than bloom threshold and output bloom colour accordingly
        float brightness = dot(Colour, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > BloomThreshold) {
            BrightColour = vec4(Colour, 1.0);
        }
        else {
            BrightColour = vec4(0.0, 0.0, 0.0, 1.0);
        }

        if (isnan(Colour.x)) {
            Colour.x = 0.0;
        }
        if (isnan(Colour.y)) {
            Colour.y = 0.0;
        }
        if (isnan(Colour.z)) {
            Colour.z = 0.0;
        }
        FragColour = vec4(Colour, 1.0);
    }
    else {
        vec3 colour = texture(nonPBRResult, TexCoords).rgb;
        vec3 bright = texture(nonPBRBrightResult, TexCoords).rgb;
        FragColour = vec4(colour, 1.0);
        BrightColour = vec4(bright, 1.0);
    }
}