#version 400 core
layout (location = 0) out vec4 FragColour;
layout (location = 1) out vec4 BrightColour;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpecular;
uniform sampler2D gPBRFLAG;
uniform sampler2D SSAO;

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

in vec3 ViewPos;

vec3 FragPos;
vec3 Colour;
vec3 Normal;
vec3 Lighting;
vec3 ViewDir;
vec3 SpecularSample;
float Shininess;
float AmbientOcclusion;

uniform float BloomThreshold;

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
    vec3 fragPos = FragPos;
    vec3 lightPos = lights[lightIndex].Position;
    float minBias = lights[lightIndex].MinShadowBias;
    float maxBias = lights[lightIndex].MaxShadowBias;
    float far_plane = lights[lightIndex].ShadowFarPlane;

    vec3 fragToLight = fragPos - lightPos;

    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias = max(maxBias * (1.0 - dot(Normal, fragToLight)), minBias);
    //float bias = 0.15;
    int samples = 20;
    
    float viewDistance = length(ViewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;

    for (int i = 0; i < samples; i++) {
        float closestDepth = texture(pointLightShadowArray, vec4(fragToLight + gridSamplingDisk[i] * diskRadius, lightIndex)).r;
        closestDepth *= far_plane;

        if (currentDepth - bias > closestDepth) {
            shadow += 1.0;
        }
    }

    shadow /= float(samples);

    return shadow;
}

vec3 BlinnPhongDirLight(DirLight light) {
    vec3 lightDir = normalize(-light.Direction);

    // diffuse
    float diff = (max(dot(Normal, lightDir), 0.0));

    //vec3 diffuse = light.Colour * diff * colour;
    vec3 diffuse = light.Colour * diff;

    // specular
    vec3 reflectDir = reflect(-lightDir, Normal);
    vec3 halfwayDir = normalize(lightDir + ViewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), Shininess);

    vec3 specular = spec * light.Colour * SpecularSample;

    // ambient
    vec3 ambient = light.Ambient * Colour * AmbientOcclusion;

    vec3 lighting;
    if (light.CastShadows) {
        // Calculate shadow
        float shadow = ShadowCalculation(light.LightSpaceMatrix * vec4(FragPos, 1.0), light.ShadowMap, -light.Direction * light.LightDistance, light.MinShadowBias, light.MaxShadowBias, light.shadowResolution, vec2(0.0, 0.0));
        lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * Colour;
    }
    else {
        diffuse = light.Colour * diff * Colour;
        lighting = diffuse + specular + ambient;
    }

    //return diffuse + specular + ambient;
    return lighting;
}

vec3 BlinnPhongSpotLight(Light light) {
    vec3 lightDir = normalize(light.Position - FragPos);
    
    // diffuse
    float diff = max(dot(Normal, lightDir), 0.0);

    vec3 diffuse = light.Colour * diff;

    // specular
    vec3 reflectDir = reflect(-lightDir, Normal);
    vec3 halfwayDir = normalize(lightDir + ViewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), Shininess); // temp shininess 16.0

    vec3 specular = spec * light.Colour * SpecularSample;
    
    // ambient
    vec3 ambient = light.Ambient * Colour * AmbientOcclusion;

    // spotLight
    float theta = dot(lightDir, normalize(-light.Direction));
    float epsilon = light.Cutoff - light.OuterCutoff;
    float intensity = clamp((theta - light.OuterCutoff) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;

    // attenuation
    float dist = length(light.Position - FragPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * dist + light.Quadratic * (dist * dist));
    diffuse *= attenuation;
    specular *= attenuation;
    ambient *= attenuation;

    vec3 lighting;
    if (light.CastShadows) {
        // Calculate shadow
        float shadow = ShadowCalculation(light.LightSpaceMatrix * vec4(FragPos, 1.0), spotlightShadowAtlas, light.Position, light.MinShadowBias, light.MaxShadowBias, light.shadowResolution, light.spotShadowAtlasTexOffset);
        lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * Colour;
    }
    else {
        diffuse *= Colour;
        lighting = diffuse + specular + ambient;
    }

    return lighting;
}

vec3 BlinnPhongPointLight(Light light, int lightIndex) {
    vec3 lightDir = normalize(light.Position - FragPos);
    
    // diffuse
    float diff = max(dot(lightDir, Normal), 0.0);

    vec3 diffuse = diff * light.Colour;

    // specular
    vec3 reflectDir = reflect(-lightDir, Normal);
    vec3 halfwayDir = normalize(lightDir + ViewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), Shininess);

    vec3 specular = spec * light.Colour * SpecularSample;

    // ambient
    vec3 ambient = light.Ambient * Colour * AmbientOcclusion;

    // attenuation
    float dist = length(light.Position - FragPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * dist + light.Quadratic * (dist * dist));
    
    diffuse *= attenuation;
    specular *= attenuation;
    ambient *= attenuation;
    
    vec3 lighting;
    if (light.CastShadows) {
        // Calculate shadow
        float shadow = CubeShadowCalculation(lightIndex);
        lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * Colour;
    }
    else {
        diffuse *= Colour;
        lighting = diffuse + specular + ambient;
    }

    return lighting;
}

uniform bool useSSAO;

void main() {
    if (texture(gPBRFLAG, TexCoords).r == 0.0) {
        // Run lighting calculations for non pbr pixel
        // Retrieve data from gBuffer
        FragPos = texture(gPosition, TexCoords).rgb;
        Normal = texture(gNormal, TexCoords).rgb;
        Colour = texture(gAlbedo, TexCoords).rgb;
        SpecularSample = texture(gSpecular, TexCoords).rgb;
        Shininess = texture(gSpecular, TexCoords).a;
        AmbientOcclusion = texture(SSAO, TexCoords).r;
        ViewDir = normalize(ViewPos - FragPos);

        if (!useSSAO) {
            AmbientOcclusion = 1.0;
        }

        // Calculate lighting as normal
        Lighting = vec3(0.0);

        // Directional Light
        Lighting += BlinnPhongDirLight(dirLight);

        // Point and spot lights
        for (int i = 0; i < activeLights && i < NR_REAL_TIME_LIGHTS; i++) {
            if (lights[i].SpotLight) {
                Lighting += BlinnPhongSpotLight(lights[i]);
            }
            else {
                Lighting += BlinnPhongPointLight(lights[i], i);
            }
        }

        // Check whether result is higher than bloom threshold and output bloom colour accordingly
        float brightness = dot(Lighting, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > BloomThreshold) {
            BrightColour = vec4(Lighting, 1.0);
        }
        else {
            BrightColour = vec4(0.0, 0.0, 0.0, 1.0);
        }

        if (isnan(Lighting.x)) {
            Lighting.x = 0.0;
        }
        if (isnan(Lighting.y)) {
            Lighting.y = 0.0;
        }
        if (isnan(Lighting.z)) {
            Lighting.z = 0.0;
        }
        FragColour = vec4(Lighting, 1.0);
    }
    else {
        FragColour = vec4(0.0, 0.0, 0.0, 1.0);
        BrightColour = vec4(0.0, 0.0, 0.0, 1.0);
    }
}