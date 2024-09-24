#version 430 core
layout (location = 0) out vec4 FragColour;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

in mat4 Projection;
in mat4 View;
in vec2 TexCoords;

// Parameters
uniform float rayStep = 0.1;
uniform float minRayStep = 0.1;
uniform float maxSteps = 30;
uniform int numBinarySearchSteps = 5;

void main() {

	FragColour =  vec4(0.0);
}