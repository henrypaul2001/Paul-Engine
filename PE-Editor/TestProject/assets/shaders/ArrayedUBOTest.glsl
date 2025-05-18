#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

layout(std140, binding = 0) uniform Camera
{
	mat4 ViewProjection;
	vec3 ViewPos;
	float Gamma;
	float Exposure;
} u_CameraBuffer;

layout(std140, binding = 1) uniform MeshSubmission
{
	mat4 Transform;
	int EntityID;
} u_MeshSubmission;

struct ChildStruct
{
	vec4 ChildPosition;
	vec4 ChildColour;
};

struct StructTest
{
	vec4 Position;
	vec4 Colour;
	ChildStruct Child[4];
};

layout(std140, binding = 2) uniform Mat_ArrayedUBOTest
{
	vec4 TestVec4;
	StructTest StructArray[8];
	vec4 AnotherTestVec4;
} u_ArrayedUBOTest;

void main()
{
	gl_Position = u_ArrayedUBOTest.StructArray[7].Position;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 colour;
layout(location = 1) out int entityID;

void main()
{
	colour = vec4(1.0);
	entityID = -1;
}

/*
	if dataType == Struct
	for (member : struct) Imple
	{
		// Add each member of the struct as it's own shader data type
		reflectionData->Add(memberName);
	}

	// ShaderDataTypes:
	// 0 "TestVec4" (vec4)
	// 1 "StructArray[0][0].Position" (vec4)
	// 1 "StructArray[0][0].Normal" (vec4)
	// 1 "StructArray[0][1].Position" (vec4)
	// 1 "StructArray[0][1].Normal" (vec4)
	// 1 "StructArray[0][2].Position" (vec4)
	// 1 "StructArray[0][2].Normal" (vec4)
	// 1 "StructArray[0][3].Position" (vec4)
	// 1 "StructArray[0][3].Normal" (vec4)
	// 1 "StructArray[0][4].Position" (vec4)
	// 1 "StructArray[0][4].Normal" (vec4)
	// 1 "StructArray[0][5].Position" (vec4)
	// 1 "StructArray[0][5].Normal" (vec4)
	// 1 "StructArray[0][6].Position" (vec4)
	// 1 "StructArray[0][6].Normal" (vec4)
	// 1 "StructArray[0][7].Position" (vec4)
	// 1 "StructArray[0][7].Normal" (vec4)
	// 2 "AnotherTestVec4" (vec4)

	// StructShaderDataType
	// Layout (data types)
	// AddDataType(ShaderDataType)

*/