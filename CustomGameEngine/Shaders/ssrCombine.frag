#version 430 core
layout (location = 0) out vec4 FragColour;
layout (location = 1) out vec4 BrightColour;

uniform sampler2D lightingPass;
uniform sampler2D ssrUVMap;

uniform sampler2D ssrReflectionMap;
uniform sampler2D ssrReflectionMapBlurred;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gArm;
uniform sampler2D gPBRFLAG;
uniform sampler2D gSpecular;

uniform sampler2D brdfLUT;

uniform float BloomThreshold;

in vec2 TexCoords;
in vec3 ViewPos;

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec4 sampleUV = texture(ssrUVMap, TexCoords);
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

    vec3 N = normalize(Normal);
    vec3 V = normalize(ViewPos - FragPos);

    vec3 reflectedColour = texture(ssrReflectionMap, TexCoords).rgb;
    float reflectionAlpha = clamp(sampleUV.b, 0.0, 1.0);
    vec3 fragSample = texture(lightingPass, TexCoords).rgb;

    vec3 F0 = vec3(0.04);

    if (texture(gPBRFLAG, TexCoords).r == 1.0) {
        float AO = texture(gArm, TexCoords).r;
        float Roughness = texture(gArm, TexCoords).g;
        float Metallic = texture(gArm, TexCoords).b;

        F0 = mix(F0, Albedo, Metallic);

        vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, Roughness);

        vec3 kS = F;
        vec3 kD = 1.0 - kS;
        kD *= 1.0 - Metallic;

        //vec3 reflectedColour = texture(lightingPass, sampleUV.xy).rgb;
        //vec3 blurredReflection = texture(ssrReflectionMapBlurred, TexCoords).rgb;
        //vec3 filteredColour = mix(reflectedColour, blurredReflection, Roughness);

        vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), Roughness)).rg;
        vec3 Specular = reflectedColour * (F * brdf.x + brdf.y);

        vec3 ambience = ((kD * Albedo + Specular) * AO);

        fragSample += ambience;
    }
    else {
        vec4 SpecularSample = texture(gSpecular, TexCoords).rgba;
        float Shininess = SpecularSample.a;

        float Roughness = 1.0 - smoothstep(0.0, 1.0, Shininess);
        vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, Roughness);
        vec3 kD = 1.0 - F;

        vec3 Specular = reflectedColour * F;

        vec3 ambience = (kD * Specular);

        fragSample += ambience;
    }
    vec3 Colour = fragSample;

    // Check whether result is higher than bloom threshold and output bloom colour accordingly
    float brightness = dot(Colour, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > BloomThreshold) {
        BrightColour = vec4(Colour, 1.0);
    }
    else {
        BrightColour = vec4(0.0, 0.0, 0.0, 1.0);
    }

    FragColour = vec4(fragSample, reflectionAlpha);
}