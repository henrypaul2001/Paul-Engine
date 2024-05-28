#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aWeights;
layout (location = 7) in mat4 aInstancedModelMatrix;
// 8, 9, 10 reserved for instancing

uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform bool instanced;

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 8;
uniform mat4 boneTransforms[MAX_BONES];

void main()
{
	mat4 Model = model;
	if (instanced) {
		Model = aInstancedModelMatrix;
	}

	vec4 transformedLocalPos = vec4(0.0);

    // Skeletal animation
    mat4 boneTransform = mat4(0.0);
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if (aBoneIDs[i] != -1 && aBoneIDs[i] < MAX_BONES) {
            boneTransform += boneTransforms[aBoneIDs[i]] * aWeights[i];
        }
    }
    transformedLocalPos = boneTransform * vec4(aPos, 1.0);

	gl_Position = lightSpaceMatrix * Model * transformedLocalPos;
}