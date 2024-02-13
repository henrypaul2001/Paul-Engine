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
};
uniform Material material;

uniform bool useDiffuseMap;
uniform bool useSpecularMap;
uniform bool useNormalMap;
uniform bool useHeightMap;

// light struct goes here

uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 viewPos;
uniform bool gamma;

vec3 BlinnPhong(vec3 normal, vec3 fragPos, vec3 lightPos, vec3 lightColor)
{
    // diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0); // 64 = shininess
    vec3 specular = spec * lightColor;

    // simple attenuation
    float max_distance = 1.5;
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (gamma ? distance * distance : distance);
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    return diffuse + specular;
}

void main()
{
    vec3 color = material.DIFFUSE;
    if (useDiffuseMap) {
        color = texture(material.TEXTURE_DIFFUSE1, fs_in.TexCoords).rgb;
    }
    //vec3 lighting = vec3(0.0);

    /*
    for (int i = 0; i < 4; ++i) {
        lighting += BlinnPhong(normalize(fs_in.Normal), fs_in.WorldPos, lightPositions[i], lightColors[i]);
    }
    
    color *= lighting;
    */

    FragColor = vec4(color, 1.0);

    /*
    if (gamma) {
        float gammaValue = 2.2;
        FragColor.rgb = pow(FragColor.rgb, vec3(1.0 / gammaValue)); // not best solution, as this would require gamma correction in every fragment shader used. Better to use as post processing on a final render quad
    }
    */
}