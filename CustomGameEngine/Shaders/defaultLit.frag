#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

// material struct
struct Material {
    sampler2D TEXTURE_DIFFUSE1;
    sampler2D TEXTURE_SPECULAR1;
    // normal map
    // height map

    float SHININESS;

    // non textured material properties
    vec3 DIFFUSE;
    vec3 SPECULAR;

    bool useDiffuseMap;
    bool useSpecularMap;
    bool useNormalMap;
    bool useHeightMap;
};
uniform Material material;

struct DirLight {
    vec3 Direction;

    vec3 Colour;
    vec3 Ambient;
    vec3 Specular;
};
uniform DirLight dirLight;

struct Light {
    vec3 Position; // universal

    vec3 Colour; // universal
    vec3 Ambient; // universal
    vec3 Specular; // universal

    float Linear; // universal
    float Quadratic; // universal
    float Constant; // universal

    bool SpotLight;
    vec3 Direction; // spotlight specific
    float Cutoff; // spotlight specific
    float OuterCutoff; // spotlight specific
};
#define NR_REAL_TIME_LIGHTS 8
uniform Light lights[NR_REAL_TIME_LIGHTS];

uniform vec3 viewPos;
uniform bool gamma;

vec3 BlinnPhongPointLight(Light light, vec3 normal, vec3 fragPos)
{
    // diffuse
    vec3 lightDir = normalize(light.Position - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 colour;
    if (!material.useDiffuseMap) {
        colour = material.DIFFUSE;
    }
    else {
        colour = texture(material.TEXTURE_DIFFUSE1, fs_in.TexCoords).rgb;
    }

    vec3 diffuse = diff * light.Colour * colour;

    // specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.SHININESS);

    vec3 specular;
    if (!material.useSpecularMap) {
        specular = spec * light.Colour * material.SPECULAR;
    }
    else {
        specular = spec * light.Colour * texture(material.TEXTURE_SPECULAR1, fs_in.TexCoords).rgb;
    }

    // ambient
    vec3 ambient = light.Ambient * colour;

    // attenuation
    float dist = length(light.Position - fragPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * dist + light.Quadratic * (dist * dist));

    // simple attenuation
    //float max_distance = 1.5;
    //float distance = length(light.Position - fragPos);
    //float attenuation = 1.0 / (gamma ? distance * distance : distance);
    
    diffuse *= attenuation;
    specular *= attenuation;
    ambient *= attenuation;
    
    return diffuse + specular + ambient;
}

vec3 BlinnPhongSpotLight(Light light, vec3 normal, vec3 fragPos) {
    // diffuse
    vec3 lightDir = normalize(light.Position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 colour;
    if (!material.useDiffuseMap) {
        colour = material.DIFFUSE;
    }
    else {
        colour = texture(material.TEXTURE_DIFFUSE1, fs_in.TexCoords).rgb;
    }
    vec3 diffuse = light.Colour * diff * colour;

    // specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.SHININESS);

    vec3 specular;
    if (!material.useSpecularMap) {
        specular = spec * light.Colour * material.SPECULAR;
    }
    else {
        specular = spec * light.Colour * texture(material.TEXTURE_SPECULAR1, fs_in.TexCoords).rgb;
    }
    
    // ambient
    vec3 ambient = light.Ambient * colour;

    // spotLight
    float theta = dot(lightDir, normalize(-light.Direction));
    float epsilon = light.Cutoff - light.OuterCutoff;
    float intensity = clamp((theta - light.OuterCutoff) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;

    // attenuation
    float dist = length(light.Position - fragPos);
    float attenuation = 1.0 / (light.Constant + light.Linear * dist + light.Quadratic * (dist * dist));
    diffuse *= attenuation;
    specular *= attenuation;
    ambient *= attenuation;

    return diffuse + specular + ambient;
}

vec3 BlinnPhongDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    // diffuse
    vec3 lightDir = normalize(-light.Direction);
    float diff = (max(dot(normal, lightDir), 0.0));
    vec3 colour;
    if (!material.useDiffuseMap) {
        colour = material.DIFFUSE;
    }
    else {
        colour = texture(material.TEXTURE_DIFFUSE1, fs_in.TexCoords).rgb;
    }
    vec3 diffuse = light.Colour * diff * colour;

    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.SHININESS);

    vec3 specular;
    if (!material.useSpecularMap) {
        specular = spec * light.Colour * material.SPECULAR;
    }
    else {
        specular = spec * light.Colour * texture(material.TEXTURE_SPECULAR1, fs_in.TexCoords).rgb;
    }

    // ambient
    vec3 ambient = light.Ambient * colour;

    return diffuse + specular + ambient;
}

void main()
{
    vec3 lighting = vec3(0.0);

    // Directional light
    lighting += BlinnPhongDirLight(dirLight, normalize(fs_in.Normal), normalize(viewPos - fs_in.WorldPos));

    // Point and spotlights
    for (int i = 0; i < lights.length(); ++i) {
        if (lights[i].SpotLight) {
            lighting += BlinnPhongSpotLight(lights[i], normalize(fs_in.Normal), fs_in.WorldPos);
        }
        else {
            lighting += BlinnPhongPointLight(lights[i], normalize(fs_in.Normal), fs_in.WorldPos);
        }
    }

    FragColor = vec4(lighting, 1.0);

    /*
    if (gamma) {
        float gammaValue = 2.2;
        FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / gammaValue)); // not best solution, as this would require gamma correction in every fragment shader used. Better to use as post processing on a final render quad
    }
    */
}