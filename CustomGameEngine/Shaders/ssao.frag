#version 330 core
out float FragColor;

in vec2 TexCoords;
in mat4 Projection;
in mat4 View;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

// SSAO parameters
uniform vec3 samples[64];
uniform float radius;
uniform int kernelSize;
uniform float bias;

uniform int scr_width;
uniform int scr_height;

// tile noise texture over screen based on screen dimensions divided by noise size
vec2 noiseScale = vec2(scr_width / 4.0, scr_height / 4.0);

void main() {
    // Get input for SSAO algorithms
    vec3 fragPos = vec3(View * vec4(texture(gPosition, TexCoords).xyz, 1.0));

    vec3 normal = mat3(View) * texture(gNormal, TexCoords).rgb;

    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    // Create TBN matrix
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal); // tangent to view space

    // iterate over sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; i++) {
        // get sample position
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * radius;

        // project sample position
        vec4 offset = vec4(samplePos, 1.0);
        offset = Projection * offset; // view to clip space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // to range 0.0, 1.0

        vec3 sampleFragPos = vec3(View * vec4(texture(gPosition, offset.xy).xyz, 1.0));
        float sampleDepth = sampleFragPos.z;

        // range check and accumulate occlusion factor
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);

    FragColor = occlusion;
}