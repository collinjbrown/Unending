#version 330

layout (location = 0) in vec3 vertPosCoords;
layout (location = 1) in vec4 vertRgbaColor;
layout (location = 2) in vec2 vertTexCoords;
layout (location = 3) in float vertTexIndex;

out vec4 rgbaColor;
out vec2 texCoords;
out float texIndex;

uniform mat4 MVP;

void main()
{
    rgbaColor = vertRgbaColor;
    texCoords = vertTexCoords;
    texIndex = vertTexIndex;
    
    gl_Position = MVP * vec4(vertPosCoords, 1.0);
}
