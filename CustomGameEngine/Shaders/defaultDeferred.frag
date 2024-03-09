#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpecular;

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

in vec3 ViewPos;

vec3 FragPos;
vec3 Colour;
vec3 Normal;
vec3 Lighting;
vec3 ViewDir;
vec3 SpecularSample;
float Shininess;

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
    vec3 lightDir = normalize(lightPos - FragPos);
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
    vec3 ambient = light.Ambient * Colour;

    vec3 lighting;
    if (light.CastShadows) {
        // Calculate shadow
        float shadow = ShadowCalculation(light.LightSpaceMatrix * vec4(FragPos, 1.0), light.ShadowMap, -light.Direction * light.LightDistance, light.MinShadowBias, light.MaxShadowBias);
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
    vec3 ambient = light.Ambient * Colour;

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
    diffuse *= Colour; // temp
    lighting = diffuse + specular + ambient; // temp
    if (light.CastShadows) {
        // Calculate shadow
        //float shadow = ShadowCalculation(light.LightSpaceMatrix * vec4(vertex_data.WorldPos, 1.0), light.ShadowMap, light.Position, light.MinShadowBias, light.MaxShadowBias);
        //lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * Colour;
    }
    else {
        //diffuse *= Colour;
        //lighting = diffuse + specular + ambient;
    }

    return lighting;
}

vec3 BlinnPhongPointLight(Light light) {
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
    vec3 ambient = light.Ambient * Colour;

    // attenuation
    float dist = length(light.Position - FragPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * dist + light.Quadratic * (dist * dist));
    
    diffuse *= attenuation;
    specular *= attenuation;
    ambient *= attenuation;
    
    vec3 lighting;
    diffuse *= Colour; // temp
    lighting = diffuse + specular + ambient; // temp
    if (light.CastShadows) {
        // Calculate shadow
        //float shadow = CubeShadowCalculation(vertex_data.WorldPos, light.CubeShadowMap, light.Position, light.MinShadowBias, light.MaxShadowBias, light.ShadowFarPlane);
        //lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * Colour;
    }
    else {
        //diffuse *= Colour;
        //lighting = diffuse + specular + ambient;
    }

    return lighting;
}

void main() {
    // Retrieve data from gBuffer
    FragPos = texture(gPosition, TexCoords).rgb;
    Normal = texture(gNormal, TexCoords).rgb;
    Colour = texture(gAlbedo, TexCoords).rgb;
    SpecularSample = texture(gSpecular, TexCoords).rgb;
    Shininess = texture(gSpecular, TexCoords).a;

    ViewDir = normalize(ViewPos - FragPos);

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
            Lighting += BlinnPhongPointLight(lights[i]);
        }
    }

    FragColor = vec4(Lighting, 1.0);
}