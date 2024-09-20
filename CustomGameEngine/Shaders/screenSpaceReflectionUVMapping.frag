#version 430 core
layout (location = 0) out vec4 FragColour;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

// Parameters
uniform float maxDistance = 15.0;
uniform float resolution = 0.3;
int steps = 10;
float thickness = 0.5;
uniform int maxRayMarchIterations = 50;

in mat4 Projection;
in mat4 View;
in vec2 TexCoords;

vec4 ViewSpaceToScreenSpace(vec4 viewSpace, vec2 texSize) {
    vec4 screenSpace = viewSpace;

    // Project
    screenSpace = Projection * screenSpace;

    // Perspective divide
    screenSpace.xyz /= screenSpace.w;

    // XY to UV
    screenSpace.xy = screenSpace.xy * 0.5 + 0.5;
    
    // UV to pixel coordinates
    screenSpace.xy *= texSize;

    return screenSpace;
}

void main() {
    vec2 texSize = textureSize(gPosition, 0).xy;
    vec2 texCoord = gl_FragCoord.xy / texSize;

    vec4 uv = vec4(0.0);

    // Retrieve fragment information from gBuffer in view space
    vec4 viewSpaceFragPos = View * texture(gPosition, texCoord);
    vec3 unitViewSpaceFragPos = normalize(viewSpaceFragPos.xyz);

    vec3 viewSpaceNormal = normalize(mat3(View) * texture(gNormal, texCoord).xyz);
    vec3 reflection = normalize(reflect(unitViewSpaceFragPos, viewSpaceNormal));

    // Record start position and end position of ray in view space
    vec4 rayStartView = vec4(viewSpaceFragPos.xyz + (reflection * 0.0), 1.0);
    vec4 rayEndView = vec4(viewSpaceFragPos.xyz + (reflection * maxDistance), 1.0);

    // Project ray start/end from view space to screen space
    vec4 rayStartScreen = ViewSpaceToScreenSpace(rayStartView, texSize);
    vec4 rayEndScreen = ViewSpaceToScreenSpace(rayEndView, texSize);

    // Prepare screen space ray march
    // ------------------------------
    vec2 frag = rayStartScreen.xy;
    uv.xy = frag / texSize;

    float deltaX = rayEndScreen.x - rayStartScreen.x;
    float deltaY = rayEndScreen.y - rayStartScreen.y;

    float useX = abs(deltaX) >= abs(deltaY) ? 1.0 : 0.0;
    float delta = mix(abs(deltaY), abs(deltaX), useX) * clamp(resolution, 0.0, 1.0);

    vec2 increment = vec2(deltaX, deltaY) / max(delta, 0.001);

    float search0 = 0;
    float search1 = 0;

    int hit0 = 0;
    int hit1 = 0;

    float viewDistance = rayStartView.z;
    float depth = thickness;

    // First ray march pass
    // --------------------
    vec4 samplePosition; // positionTo
    for (int i = 0; i < min(int(delta), maxRayMarchIterations); i++) {
        frag += increment;
        uv.xy = frag / texSize;
        samplePosition = View * texture(gPosition, uv.xy);

        search1 = mix((frag.y - rayStartScreen.y) / deltaY, (frag.x - rayStartScreen.x) / deltaX, useX);
        search1 = clamp(search1, 0.0, 1.0);

        // Interpolate view distance
        viewDistance = (rayStartView.y * rayEndView.y) / mix(rayEndView.y, rayStartView.y, search1);
        depth = viewDistance - samplePosition.y;

        // Check for hit
        if (depth > 0 && depth < thickness) {
            hit0 = 1;
            break;
        }
        else {
            search0 = search1;
        }
    }

    // Set search1 position to halfway between last miss and last hit
    search1 = search0 + ((search1 - search0) / 2.0);

    // Second refinement pass
    // ----------------------
    steps *= hit0;
    for (int i = 0; i < steps; i++) {
        frag = mix(rayStartScreen.xy, rayEndScreen.xy, search1);
        uv.xy = frag / texSize;
        samplePosition = View * texture(gPosition, uv.xy);

        viewDistance = (rayStartView.y * rayEndView.y) / mix(rayEndView.y, rayStartView.y, search1);
        depth = viewDistance - samplePosition.y;

        // Check for hit
        if (depth > 0 && depth < thickness) {
            hit1 = 1;
            search1 = search0 + ((search1 - search0) / 2.0);
        }
        else {
            float temp = search1;
            search1 = search1 + ((search1 - search0) / 2.0);
            search0 = temp;
        }
    }

    // Calculate visibility
    float visibility = hit1 * samplePosition.w
                    * (1 - max(dot(unitViewSpaceFragPos, reflection), 0.0))                             // Fade if pointing towards camera
                    * (1 - clamp(depth / thickness, 0.0, 1.0))                                          // Fade reflection the further away from intersect point
                    * (1 - clamp(length(samplePosition - viewSpaceFragPos) / maxDistance, 0.0, 1.0))    // Fade based on distance to initial ray start point
                    * (uv.x < 0.0 || uv.x > 1.0 ? 0.0 : 1.0)                                            // Out of bounds checks
                    * (uv.y < 0.0 || uv.y > 1.0 ? 0.0 : 1.0);

    visibility = clamp(visibility, 0.0, 1.0);
    uv.ba = vec2(visibility);

    FragColour = uv;
}