#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

// uniform block
layout (std140) uniform Common
{
    mat4 projection;
    mat4 view;
    vec3 viewPos;
};

out mat4 Projection;
out mat4 View;
out vec2 TexCoords;

void main()
{
    View = view;
    Projection = projection;
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}