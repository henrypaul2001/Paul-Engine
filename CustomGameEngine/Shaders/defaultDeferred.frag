#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

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

uniform vec3 viewPos;

vec3 FragPos;
vec3 Colour;
vec3 Normal;
vec3 Lighting;
float Shininess;

vec3 BlinnPhongDirLight(DirLight light, vec3 viewDir) {
    vec3 lightDir = normalize(-light.Direction);

    // diffuse
    float diff = (max(dot(Normal, lightDir), 0.0));

    //vec3 diffuse = light.Colour * diff * colour;
    vec3 diffuse = light.Colour * diff;

    // specular
    vec3 reflectDir = reflect(-lightDir, Normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), Shininess);

    vec3 specular = spec * light.Colour * Shininess;

    // ambient
    vec3 ambient = light.Ambient * Colour;

    vec3 lighting;
    diffuse = light.Colour * diff * Colour; // temp
    lighting = diffuse + specular + ambient; // temp
    if (light.CastShadows) {
        // Calculate shadow
        //float shadow = ShadowCalculation(light.LightSpaceMatrix * vec4(vertex_data.WorldPos, 1.0), light.ShadowMap, -light.Direction * light.LightDistance, light.MinShadowBias, light.MaxShadowBias);
        //lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * Colour;
    }
    else {
        //diffuse = light.Colour * diff * Colour;
        //lighting = diffuse + specular + ambient;
    }

    //return diffuse + specular + ambient;
    return lighting;
}

void main() {
    // Retrieve data from gBuffer
    FragPos = texture(gPosition, TexCoords).rgb;
    Normal = texture(gNormal, TexCoords).rgb;
    Colour = texture(gAlbedoSpec, TexCoords).rgb;
    Shininess = texture(gAlbedoSpec, TexCoords).a;

    vec3 viewDir = normalize(viewPos - FragPos);

    // Calculate lighting as normal
    Lighting = vec3(0.0);

    // Directional Light
    Lighting += BlinnPhongDirLight(dirLight, viewDir);

    // Point and spot lights
    for (int i = 0; i < activeLights && i < NR_REAL_TIME_LIGHTS; i++) {
        if (lights[i].SpotLight) {
            //Lighting += BlinnPhongSpotLight(lights[i]);
        }
        else {
            //Lighting += BlinnPhongPointLight(lights[i]);
        }
    }

    FragColor = vec4(Lighting, 1.0);
}