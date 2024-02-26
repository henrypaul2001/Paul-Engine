#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

// material struct
struct Material {
    sampler2D TEXTURE_DIFFUSE1;
    sampler2D TEXTURE_SPECULAR1;
    sampler2D TEXTURE_NORMAL1;
    sampler2D TEXTURE_DISPLACE1;

    float SHININESS;
    float HEIGHT_SCALE;

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
    vec3 TangentDirection;
    
    mat4 LightSpaceMatrix;
    sampler2D ShadowMap;

    vec3 Colour;
    vec3 Ambient;
    vec3 Specular;
};
uniform DirLight dirLight;

struct Light {
    vec3 Position; // universal
    vec3 TangentPosition;

    vec3 Colour; // universal
    vec3 Ambient; // universal
    vec3 Specular; // universal

    float Linear; // universal
    float Quadratic; // universal
    float Constant; // universal

    bool SpotLight;
    vec3 Direction; // spotlight specific
    vec3 TangentDirection;
    float Cutoff; // spotlight specific
    float OuterCutoff; // spotlight specific
};
#define NR_REAL_TIME_LIGHTS 8
uniform Light lights[NR_REAL_TIME_LIGHTS];
uniform int activeLights;

uniform vec3 viewPos;

vec3 TangentViewPos;
vec3 TangentFragPos;
vec3 TangentViewDirection;

uniform bool gamma;

uniform float textureScale;

vec2 TexCoords;
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

float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap, vec3 lightPos) {
    // perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0, 1]
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0) {
        return 0.0;
    }
    
    // get closes depth value from lights perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // get depth of current fragment
    float currentDepth = projCoords.z;

    // check if current frag pos is in shadow
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.WorldPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

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

vec3 BlinnPhongPointLight(Light light, vec3 normal, vec3 fragPos) {
    light.TangentPosition = fs_in.TBN * light.Position;
    vec3 lightDir = normalize(light.TangentPosition - fragPos); // light.Position
    
    // diffuse
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 colour;
    if (!material.useDiffuseMap) {
        colour = material.DIFFUSE;
    }
    else {
        colour = texture(material.TEXTURE_DIFFUSE1, TexCoords).rgb;
    }

    vec3 diffuse = diff * light.Colour * colour;

    // specular
    //vec3 viewDir = normalize(viewPos - fragPos);
    //vec3 TangentViewDir = normalize(TangentViewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + TangentViewDirection);  //TangentViewDirection
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.SHININESS);

    vec3 specular;
    if (!material.useSpecularMap) {
        specular = spec * light.Colour * material.SPECULAR;
    }
    else {
        specular = spec * light.Colour * texture(material.TEXTURE_SPECULAR1, TexCoords).rgb;
    }

    // ambient
    vec3 ambient = light.Ambient * colour;

    // attenuation
    float dist = length(light.TangentPosition - fragPos); // light.Position
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
    light.TangentDirection = fs_in.TBN * light.Direction;
    light.TangentPosition = fs_in.TBN * light.Position;

    vec3 lightDir = normalize(light.TangentPosition - fragPos); //light.Position - fragPos
    
    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 colour;
    if (!material.useDiffuseMap) {
        colour = material.DIFFUSE;
    }
    else {
        colour = texture(material.TEXTURE_DIFFUSE1, TexCoords).rgb;
    }
    vec3 diffuse = light.Colour * diff * colour;

    // specular
    //vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + TangentViewDirection);  // tangent
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.SHININESS);

    vec3 specular;
    if (!material.useSpecularMap) {
        specular = spec * light.Colour * material.SPECULAR;
    }
    else {
        specular = spec * light.Colour * texture(material.TEXTURE_SPECULAR1, TexCoords).rgb;
    }
    
    // ambient
    vec3 ambient = light.Ambient * colour;

    // spotLight
    float theta = dot(lightDir, normalize(-light.TangentDirection)); // tangent
    float epsilon = light.Cutoff - light.OuterCutoff;
    float intensity = clamp((theta - light.OuterCutoff) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;

    // attenuation
    float dist = length(light.TangentPosition - fragPos); // light.Position
    float attenuation = 1.0 / (light.Constant + light.Linear * dist + light.Quadratic * (dist * dist));
    diffuse *= attenuation;
    specular *= attenuation;
    ambient *= attenuation;

    return diffuse + specular + ambient;
}

vec3 BlinnPhongDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    light.TangentDirection = fs_in.TBN * light.Direction;
    vec3 lightDir = normalize(-light.TangentDirection); // -light.Direction

    // diffuse
    float diff = (max(dot(normal, lightDir), 0.0));
    vec3 colour;
    if (!material.useDiffuseMap) {
        colour = material.DIFFUSE;
    }
    else {
        colour = texture(material.TEXTURE_DIFFUSE1, TexCoords).rgb;
    }
    //vec3 diffuse = light.Colour * diff * colour;
    vec3 diffuse = light.Colour * diff;

    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.SHININESS);

    vec3 specular;
    if (!material.useSpecularMap) {
        specular = spec * light.Colour * material.SPECULAR;
    }
    else {
        specular = spec * light.Colour * texture(material.TEXTURE_SPECULAR1, TexCoords).rgb;
    }

    // ambient
    vec3 ambient = light.Ambient * colour;

    // Calculate shadow
    float shadow = ShadowCalculation(dirLight.LightSpaceMatrix * vec4(fs_in.WorldPos, 1.0), dirLight.ShadowMap, -dirLight.Direction);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * colour;

    //return diffuse + specular + ambient;
    return lighting;
}

vec3 GetNormalFromMap() {
    vec3 tangentNormal = texture(material.TEXTURE_NORMAL1, TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1 = dFdx(fs_in.WorldPos);
    vec3 Q2  = dFdy(fs_in.WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N = normalize(fs_in.Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
    // vs_out.TangentLightPos = TBN * lightPos;
    // vs_out.TangentViewPos  = TBN * viewPos;
    // vs_out.TangentFragPos  = TBN * vs_out.FragPos;
    TangentViewPos = fs_in.TBN * viewPos;
    TangentFragPos = fs_in.TBN * fs_in.WorldPos;
    TangentViewDirection = normalize(TangentViewPos - TangentFragPos);

    TexCoords = fs_in.TexCoords;
    TexCoords *= textureScale;
    if (material.useHeightMap) {
        TexCoords = ParallaxMapping(TexCoords, TangentViewDirection); //normalize(viewPos - fs_in.WorldPos)
        if (TexCoords.x > 1.0 || TexCoords.y > 1.0 || TexCoords.x < 0.0 || TexCoords.y < 0.0) {
            //discard;
        }
    }
    //TexCoords *= textureScale;

    vec3 lighting = vec3(0.0);

    vec3 normal = normalize(fs_in.Normal);
    if (material.useNormalMap) {
        //normal = GetNormalFromMap();
        normal = normalize(texture(material.TEXTURE_NORMAL1, TexCoords)).rgb;
    }
    //normal = normalize(normal * 2.0 - 1.0); // tangent space
    //normal = normalize(fs_in.TBN * normal);

    // Directional light
    lighting += BlinnPhongDirLight(dirLight, normalize(fs_in.TBN * normal), TangentViewDirection); // normalize(viewPos - fs_in.WorldPos)

    // Point and spotlights
    for (int i = 0; i < activeLights; i++) {
        if (lights[i].SpotLight) {
            lighting += BlinnPhongSpotLight(lights[i], normalize(normal * 2.0 - 1.0), TangentFragPos); // fs_in.WorldPos
        }
        else {
            lighting += BlinnPhongPointLight(lights[i], normalize(fs_in.TBN * normal), TangentFragPos); // fs_in.WorldPos
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