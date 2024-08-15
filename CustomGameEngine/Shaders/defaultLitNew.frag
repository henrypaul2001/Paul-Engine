#version 400 core
layout (location = 0) out vec4 FragColour;
layout (location = 1) out vec4 BrightColour;

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

struct Material {
    sampler2D TEXTURE_DIFFUSE1;
    sampler2D TEXTURE_SPECULAR1;
    sampler2D TEXTURE_NORMAL1;
    sampler2D TEXTURE_DISPLACE1;
    sampler2D TEXTURE_OPACITY1;

    float SHININESS;
    float HEIGHT_SCALE;

    // non textured material properties
    vec3 DIFFUSE;
    vec3 SPECULAR;

    bool useDiffuseMap;
    bool useSpecularMap;
    bool useNormalMap;
    bool useHeightMap;
    bool useOpacityMap;
};
uniform Material material;

uniform vec2 textureScale;

vec2 TexCoords;
vec3 Colour;
vec3 Normal;
vec3 Lighting;
vec3 SpecularSample;
vec3 TangentViewDirection;
float Alpha;

uniform float BloomThreshold;

uniform bool OpaqueRenderPass;

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
    vec3 lightDir = normalize(lightPos - vertex_data.WorldPos);
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
    vec3 fragPos = vertex_data.WorldPos;
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
    
    float viewDistance = length(view_data.ViewPos - fragPos);
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

vec3 BlinnPhongDirLight(DirLight light, vec3 viewDir) {
    light.TangentDirection = vertex_data.TBN * light.Direction;
    vec3 lightDir = normalize(-light.TangentDirection);

    // diffuse
    float diff = (max(dot(Normal, lightDir), 0.0));

    //vec3 diffuse = light.Colour * diff * colour;
    vec3 diffuse = light.Colour * diff;

    // specular
    vec3 reflectDir = reflect(-lightDir, Normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), material.SHININESS);

    vec3 specular = spec * light.Colour * SpecularSample;

    // ambient
    vec3 ambient = light.Ambient * Colour;

    vec3 lighting;
    if (light.CastShadows) {
        // Calculate shadow
        float shadow = ShadowCalculation(light.LightSpaceMatrix * vec4(vertex_data.WorldPos, 1.0), light.ShadowMap, -light.Direction * light.LightDistance, light.MinShadowBias, light.MaxShadowBias, light.shadowResolution, vec2(0.0, 0.0));
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
    light.TangentDirection = vertex_data.TBN * light.Direction;
    light.TangentPosition = vertex_data.TBN * light.Position;
    vec3 lightDir = normalize(light.TangentPosition - vertex_data.TangentFragPos);
    
    // diffuse
    float diff = max(dot(Normal, lightDir), 0.0);

    vec3 diffuse = light.Colour * diff;

    // specular
    vec3 reflectDir = reflect(-lightDir, Normal);
    vec3 halfwayDir = normalize(lightDir + TangentViewDirection);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), material.SHININESS);

    vec3 specular = spec * light.Colour * SpecularSample;
    
    // ambient
    vec3 ambient = light.Ambient * Colour;

    // spotLight
    float theta = dot(lightDir, normalize(-light.TangentDirection));
    float epsilon = light.Cutoff - light.OuterCutoff;
    float intensity = clamp((theta - light.OuterCutoff) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;

    // attenuation
    float dist = length(light.TangentPosition - vertex_data.TangentFragPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * dist + light.Quadratic * (dist * dist));
    diffuse *= attenuation;
    specular *= attenuation;
    ambient *= attenuation;

    vec3 lighting;
    if (light.CastShadows) {
        // Calculate shadow
        float shadow = ShadowCalculation(light.LightSpaceMatrix * vec4(vertex_data.WorldPos, 1.0), spotlightShadowAtlas, light.Position, light.MinShadowBias, light.MaxShadowBias, light.shadowResolution, light.spotShadowAtlasTexOffset);
        lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * Colour;
    }
    else {
        diffuse *= Colour;
        lighting = diffuse + specular + ambient;
    }

    return lighting;
}

vec3 BlinnPhongPointLight(Light light, int lightIndex) {
    light.TangentPosition = vertex_data.TBN * light.Position;
    vec3 lightDir = normalize(light.TangentPosition - vertex_data.TangentFragPos);
    
    // diffuse
    float diff = max(dot(lightDir, Normal), 0.0);

    vec3 diffuse = diff * light.Colour;

    // specular
    vec3 reflectDir = reflect(-lightDir, Normal);
    vec3 halfwayDir = normalize(lightDir + TangentViewDirection);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), material.SHININESS);

    vec3 specular = spec * light.Colour * SpecularSample;

    // ambient
    vec3 ambient = light.Ambient * Colour;

    // attenuation
    float dist = length(light.TangentPosition - vertex_data.TangentFragPos);
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

void CalculateLighting() {
    // Get base colour
    Colour = material.DIFFUSE;
    if (material.useDiffuseMap) {
        Colour = texture(material.TEXTURE_DIFFUSE1, TexCoords).rgb;
    }

    // Get specular sample
    SpecularSample = material.SPECULAR;
    if (material.useSpecularMap) {
        SpecularSample = texture(material.TEXTURE_SPECULAR1, TexCoords).rgb;
    }

    // Get normal sample
    Normal = normalize(vertex_data.Normal);
    if (material.useNormalMap) {
        Normal = normalize(texture(material.TEXTURE_NORMAL1, TexCoords)).rgb;
    }
    //Normal = normalize(Normal * 2.0 - 1.0); // tangent space
    Normal = normalize(vertex_data.TBN * Normal);

    Lighting = vec3(0.0);

    // Directional Light
    Lighting += BlinnPhongDirLight(dirLight, TangentViewDirection);

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

    FragColour = vec4(Lighting, Alpha);
}

void main() {
    bool opaquePixel = false;

    TexCoords = vertex_data.TexCoords;
    TexCoords *= textureScale;

    TangentViewDirection = normalize(view_data.TangentViewPos - vertex_data.TangentFragPos);

    // Apply parallax mapping to tex coords if material has height map
    if (material.useHeightMap) {
        TexCoords = ParallaxMapping(TexCoords, TangentViewDirection);
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