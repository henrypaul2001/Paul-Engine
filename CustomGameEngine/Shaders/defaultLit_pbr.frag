#version 330 core
out vec4 FragColor;

#define NR_REAL_TIME_LIGHTS 8
struct DirLight {
    vec3 Direction;
    vec3 TangentDirection;
    
    mat4 LightSpaceMatrix;
    float LightDistance;

    bool CastShadows;

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

in VIEW_DATA {
    flat vec3 TangentViewPos;
    flat vec3 ViewPos;
} view_data;

in VERTEX_DATA {
	vec3 WorldPos;
	vec3 Normal;
	vec2 TexCoords;

    mat3 TBN;

	vec3 TangentFragPos;
} vertex_data;

struct PBRMaterial {
    sampler2D TEXTURE_ALBEDO1;
    sampler2D TEXTURE_NORMAL1;
    sampler2D TEXTURE_METALLIC1;
    sampler2D TEXTURE_ROUGHNESS1;
    sampler2D TEXTURE_AO1;


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
};
uniform PBRMaterial material;

uniform float textureScale;

vec3 Albedo;
vec3 Normal;
float Metallic;
float Roughness;
float AO;

vec3 N;
vec3 V;
vec3 R;
vec3 F0;

const float PI = 3.14159265359;

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

vec3 PerLightReflectance_PointLight(int lightIndex) {
    // Radiance
    vec3 L = normalize(lights[lightIndex].Position - vertex_data.WorldPos);
    vec3 H = normalize(V + L);

    float dist = length(lights[lightIndex].Position - vertex_data.WorldPos);
    float attenuation = 1.0 / (dist * dist);
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

    // Add to outgoing radiance Lo
    return (kD * Albedo / PI + specular) * radiance * NdotL;
}

vec3 GetNormalFromMap() {
    vec3 tangentNormal = texture(material.TEXTURE_NORMAL1, vertex_data.TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(vertex_data.WorldPos);
    vec3 Q2 = dFdy(vertex_data.WorldPos);
    vec2 st1 = dFdx(vertex_data.TexCoords);
    vec2 st2 = dFdy(vertex_data.TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main() {
	// Get material colour
    Albedo = material.ALBEDO;
    if (material.useAlbedoMap) {
        Albedo = texture(material.TEXTURE_ALBEDO1, vertex_data.TexCoords).rgb;
        //Albedo = pow(texture(material.TEXTURE_ALBEDO1, vertex_data.TexCoords).rgb, vec3(2.2)); // tone mapped version
    }

    // Get fragment normal
    Normal = vertex_data.Normal;
    if (material.useNormalMap) {
        Normal = GetNormalFromMap();
    }

    // Get material metalness value
    Metallic = material.METALNESS;
    if (material.useMetallicMap) {
        Metallic = texture(material.TEXTURE_METALLIC1, vertex_data.TexCoords).r;
    }

    // Get material roughness value
    Roughness = material.ROUGHNESS;
    if (material.useRoughnessMap) {
        Roughness = texture(material.TEXTURE_ROUGHNESS1, vertex_data.TexCoords).r;
    }

    // Get fragment ambient occlusion factor
    AO = material.AO;
    if (material.useAoMap) {
        AO = texture(material.TEXTURE_AO1, vertex_data.TexCoords).r;
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

    // Lo += DirLightReflectanceLO();
    for (int i = 0; i < activeLights && i < NR_REAL_TIME_LIGHTS; i++) {
        if (lights[i].SpotLight) {
            //Lo += BlinnPhongSpotLight(lights[i]);
        }
        else {
            Lo += PerLightReflectance_PointLight(i);
        }
    }

    // Ambient lighting
    vec3 ambient = vec3(0.01) * Albedo * AO;
    /*
    if (useIBL) {
        // Image based lighting goes here
    }
    */

    vec3 Colour = ambient + Lo;

    // Tone mapping
    //Colour = Colour / (Colour + vec3(1.0));

    // Gamma correction
    //Colour = pow(Colour, vec3(1.0 / 2.2));

    FragColor = vec4(Colour, 1.0);
}