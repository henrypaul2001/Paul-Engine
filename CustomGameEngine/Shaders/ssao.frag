#version 330 core
out float FragColor;

in vec2 TexCoords;

// uniform block
layout (std140) uniform Common
{
    mat4 projection;
    mat4 view;
    vec3 viewPos;
};

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

// SSAO parameters
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

uniform int scr_width;
uniform int scr_height;

// tile noise texture over screen based on screen dimensions divided by noise size
vec2 noiseScale = vec2(scr_width / 4.0, scr_height / 4.0);

void main() {
    // Get input for SSAO algorithms
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    // Create TBN matrix
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // iterate over sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; i++) {
        // get sample position
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * radius;

        // project sample position
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // view to clip space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // to range 0.0, 1.0

        // get sample depth
        float sampleDepth = texture(gPosition, offset.xy).z;

        // range check and accumulate occlusion factor
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);

    FragColor = occlusion;
}