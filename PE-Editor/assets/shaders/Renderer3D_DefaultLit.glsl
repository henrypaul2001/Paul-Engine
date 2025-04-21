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
} u_CameraBuffer;

layout(std140, binding = 1) uniform MeshSubmission
{
	mat4 Transform;
	int EntityID;
} u_MeshSubmission;

struct ViewData
{
	vec3 ViewPos;
};

struct VertexData
{
	vec3 WorldFragPos;
	vec3 Normal;
	vec2 TexCoords;

	mat3 TBN;
};

layout(location = 0) out flat int v_EntityID;
layout(location = 1) out flat ViewData v_ViewData;
layout(location = 3) out VertexData v_VertexData;

void main()
{
	mat3 normalMatrix = mat3(transpose(inverse(u_MeshSubmission.Transform)));
	v_VertexData.WorldFragPos = vec3(u_MeshSubmission.Transform * vec4(a_Position, 1.0));
	v_VertexData.Normal = normalMatrix * a_Normal;
	v_VertexData.TexCoords = a_TexCoords;

	// Tangent space
	vec3 T = normalMatrix * a_Tangent;
	vec3 N = v_VertexData.Normal;
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	v_VertexData.TBN = mat3(T, B, N);

	v_ViewData.ViewPos = u_CameraBuffer.ViewPos;

	v_EntityID = u_MeshSubmission.EntityID;

	gl_Position = u_CameraBuffer.ViewProjection * vec4(v_VertexData.WorldFragPos, 1.0);
}

#type fragment
#version 450 core
layout(location = 0) out vec4 colour;
layout(location = 1) out int entityID;

struct ViewData
{
	vec3 ViewPos;
};

struct VertexData
{
	vec3 WorldFragPos;
	vec3 Normal;
	vec2 TexCoords;

	mat3 TBN;
};

struct DirectionalLight
{
	vec3 Direction;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};

layout(location = 0) in flat int v_EntityID;
layout(location = 1) in flat ViewData v_ViewData;
layout(location = 3) in VertexData v_VertexData;

layout(std140, binding = 2) uniform MaterialValues
{
	vec4 Albedo;
	vec4 Specular;
	float Shininess;
	int UseNormalMap;
	vec2 TextureScale;
} u_MaterialValues;

layout(binding = 0) uniform sampler2D AlbedoMap;
layout(binding = 1) uniform sampler2D SpecularMap;
layout(binding = 2) uniform sampler2D NormalMap;

vec2 ScaledTexCoords;

void main()
{
	ScaledTexCoords = v_VertexData.TexCoords;
	//ScaledTexCoords *= u_MaterialValues.TextureScale;

	DirectionalLight light;
	light.Direction = vec3(-0.2, -0.5, -0.3);
	light.Ambient = vec3(0.2, 0.2, 0.2);
	light.Diffuse = vec3(0.5, 0.5, 0.5);
	light.Specular = vec3(1.0, 1.0, 1.0);

	vec3 AlbedoSample = texture(AlbedoMap, ScaledTexCoords).rgb;
	vec3 SpecularSample = vec3(texture(SpecularMap, ScaledTexCoords).r);

	vec3 Normal = normalize(v_VertexData.Normal);
	if (u_MaterialValues.UseNormalMap != 0)
	{
		Normal = texture(NormalMap, ScaledTexCoords).rgb;
		Normal = normalize(Normal * 2.0 - 1.0);
		Normal = normalize(v_VertexData.TBN * Normal);
	}

	vec3 MaterialAlbedo = AlbedoSample * u_MaterialValues.Albedo.rgb;
	vec3 MaterialSpecular = SpecularSample * u_MaterialValues.Specular.rgb;

	vec3 ambient = light.Ambient * MaterialAlbedo;

	// diffuse
	vec3 lightDir = normalize(-light.Direction);
	float diff = max(dot(Normal, lightDir), 0.0);
	vec3 diffuse = light.Diffuse * diff * MaterialAlbedo;

	// specular
	vec3 viewDir = normalize(v_ViewData.ViewPos - v_VertexData.WorldFragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(Normal, halfwayDir), 0.0), u_MaterialValues.Shininess);
	vec3 specular = (light.Specular * spec * MaterialSpecular) * diff;

	vec3 result = ambient + diffuse + specular;
	colour = vec4(result, u_MaterialValues.Albedo.a);

	if (colour.a == 0.0) { discard; }
	else { entityID = v_EntityID; }
}