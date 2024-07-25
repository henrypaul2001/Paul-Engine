#version 330 core
layout (location = 0) out vec4 FragColour;

// -------------|  Lights  |-----------------
// ------------------------------------------
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

    sampler2D ShadowMap; // spot light specific
    samplerCube CubeShadowMap; // point light specific

    bool SpotLight;
    vec3 Direction; // spotlight specific
    vec3 TangentDirection; // spotlight specific

    float Cutoff; // spotlight specific
    float OuterCutoff; // spotlight specific
};

uniform DirLight dirLight;
uniform Light lights[NR_REAL_TIME_LIGHTS];
uniform int activeLights;

// -------------|   INPUT  |-----------------
// ------------------------------------------
in GEOMETRY_VIEW_OUTPUT {
    flat vec3 TangentViewPos;
    flat vec3 ViewPos;
} view_data;

in GEOMETRY_VERT_OUTPUT {
	vec3 WorldPos;
	vec3 Normal;
	vec2 TexCoords;

    mat3 TBN;

	vec3 TangentFragPos;
} vertex_data;

// -------------| Material |-----------------
// ------------------------------------------
struct PBRMaterial {
    sampler2D TEXTURE_ALBEDO1;
    sampler2D TEXTURE_NORMAL1;
    sampler2D TEXTURE_METALLIC1;
    sampler2D TEXTURE_ROUGHNESS1;
    sampler2D TEXTURE_AO1;
    sampler2D TEXTURE_DISPLACE1;
    sampler2D TEXTURE_OPACITY1;

    float HEIGHT_SCALE;

    // non textured material properties
    vec3 ALBEDO;
    float METALNESS;
    float ROUGHNESS;
    float AO;

    bool useAlbedoMap;
    bool useNormalMap;
    bool useMetallicMap;
    bool useRoughnessMap;
    bool useAoMap;
    bool useHeightMap;
    bool useOpacityMap;
};
uniform PBRMaterial material;

vec3 Albedo;
vec3 Normal;
float Metallic;
float Roughness;
float AO;
float Alpha;
vec2 TexCoords;

vec3 N;
vec3 V;
vec3 R;
vec3 F0;

uniform float BloomThreshold;
uniform bool OpaqueRenderPass;

const float PI = 3.14159265359;

const float minLayers = 8.0;
const float maxLayers = 32.0;
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
    // number of depth layers
    float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));

    // calculate size of each layer
    float layerDepth = 1.0 / numLayers;

    float currentLayerDepth = 0.0;

    // amount to shift the texture coordinates per layer
    vec2 P = viewDir.xy * material.HEIGHT_SCALE;
    vec2 deltaTexCoords = (P / numLayers);

    // get initial values
    vec2 currentTexCoords = texCoords;
    float currentDepthMapValue = texture(material.TEXTURE_DISPLACE1, currentTexCoords).r;

    while (currentLayerDepth < currentDepthMapValue) {
        // shift coords along direction of P
        currentTexCoords -= deltaTexCoords;

        // get new depth value
        currentDepthMapValue = texture(material.TEXTURE_DISPLACE1, currentTexCoords).r;

        // get depth of next layer
        currentLayerDepth += layerDepth;
    }
    
    // get texture coordinates before collision
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(material.TEXTURE_DISPLACE1, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texCoords
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap, vec3 lightPos, float minBias, float maxBias) {
    // perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0, 1]
    projCoords.xyz = projCoords.xyz * 0.5 + 0.5;

    if (projCoords.z > 1.0) {
        return 0.0;
    }
    
    // get closes depth value from lights perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // get depth of current fragment
    float currentDepth = projCoords.z;

    // check if current frag pos is in shadow
    vec3 lightDir = normalize(lightPos - vertex_data.WorldPos);
    float bias = max(maxBias * (1.0 - dot(Normal, lightDir)), minBias);

    // pcf soft shadows (simple solution but more advanced solutions out there)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
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
    vec3 fragToLight = vertex_data.WorldPos - lights[lightIndex].Position;

    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = max(lights[lightIndex].MaxShadowBias * (1.0 - dot(Normal, fragToLight)), lights[lightIndex].MinShadowBias);
    //float bias = 0.15;
    int samples = 20;
    
    float viewDistance = length(view_data.ViewPos - vertex_data.WorldPos);
    float diskRadius = (1.0 + (viewDistance / lights[lightIndex].ShadowFarPlane)) / 25.0;

    for (int i = 0; i < samples; i++) {
        float closestDepth = texture(lights[lightIndex].CubeShadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
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

float GeometrySchlickGGX(float NdotV)
{
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
    vec3 L = normalize(lights[lightIndex].Position - vertex_data.WorldPos);
    vec3 H = normalize(V + L);

    float dist = length(lights[lightIndex].TangentPosition - vertex_data.TangentFragPos);
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
        float shadow = ShadowCalculation(lights[lightIndex].LightSpaceMatrix * vec4(vertex_data.WorldPos, 1.0), lights[lightIndex].ShadowMap, lights[lightIndex].Position, lights[lightIndex].MinShadowBias, lights[lightIndex].MaxShadowBias);
        kD *= (1.0 - shadow);
        specular *= (1.0 - shadow);
        radiance *= (1.0 - shadow);
        NdotL *= (1.0 - shadow);
    }

    // Add to outgoing radiance Lo
    vec3 Lo = (kD * Albedo / PI + specular) * radiance * NdotL;
    return Lo;
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
        float shadow = ShadowCalculation(dirLight.LightSpaceMatrix * vec4(vertex_data.WorldPos, 1.0), dirLight.ShadowMap, -dirLight.Direction * dirLight.LightDistance, dirLight.MinShadowBias, dirLight.MaxShadowBias);
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
    vec3 L = normalize(lights[lightIndex].Position - vertex_data.WorldPos);
    vec3 H = normalize(V + L);

    float dist = length(lights[lightIndex].Position - vertex_data.WorldPos);
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

vec3 GetNormalFromMap() {
    vec3 tangentNormal = texture(material.TEXTURE_NORMAL1, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(vertex_data.WorldPos);
    vec3 Q2 = dFdy(vertex_data.WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void CalculateLighting() {
    // Get material colour
    Albedo = material.ALBEDO;
    if (material.useAlbedoMap) {
        Albedo = texture(material.TEXTURE_ALBEDO1, TexCoords).rgb;
    }

    // Get fragment normal
    Normal = vertex_data.Normal;
    if (material.useNormalMap) {
        Normal = GetNormalFromMap();
    }

    // Get material metalness value
    Metallic = material.METALNESS;
    if (material.useMetallicMap) {
        Metallic = texture(material.TEXTURE_METALLIC1, TexCoords).r;
    }

    // Get material roughness value
    Roughness = material.ROUGHNESS;
    if (material.useRoughnessMap) {
        Roughness = texture(material.TEXTURE_ROUGHNESS1, TexCoords).r;
    }

    // Get fragment ambient occlusion factor
    AO = material.AO;
    if (material.useAoMap) {
        AO = texture(material.TEXTURE_AO1, TexCoords).r;
    }

    N = normalize(Normal);
    V = normalize(view_data.ViewPos - vertex_data.WorldPos);
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
    vec3 ambient;

    if (dirLight.Active) {
        ambient = dirLight.Ambient * Albedo * AO;
    }
    else {
        ambient = vec3(0.01) * Albedo * AO;
    }

    vec3 Colour = ambient + Lo;

    FragColour = vec4(Colour, Alpha);
}

void main() {
    bool opaquePixel = false;
    TexCoords = vertex_data.TexCoords;
    // Apply parallax mapping to tex coords if material has height map
    if (material.useHeightMap) {
        vec3 viewDir = normalize(view_data.TangentViewPos - vertex_data.TangentFragPos);
        TexCoords = ParallaxMapping(TexCoords, viewDir);
        if (TexCoords.x > 1.0 || TexCoords.y > 1.0 || TexCoords.x < 0.0 || TexCoords.y < 0.0) {
            //discard;
        }
    }

    // Get alpha
    Alpha = 1.0;
    if (material.useOpacityMap) {
        Alpha = texture(material.TEXTURE_OPACITY1, TexCoords).a;
    }

    if (Alpha == 1.0) {
        opaquePixel = true;
    }
    
    // First pass will render fully opaque pixels. Second pass will render non opaque pixels
    if (OpaqueRenderPass && opaquePixel) {
        CalculateLighting();
    }
    else if (!OpaqueRenderPass && !opaquePixel) {
        CalculateLighting();
    }
    else {
        discard;
    }
}