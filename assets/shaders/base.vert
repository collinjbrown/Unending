#version 330

layout (location = 0) in vec3 vertPosCoords;
layout (location = 1) in vec4 vertRgbaColor;

out vec4 rgbaColor;

uniform mat4 MVP;

void main()
{
    rgbaColor = vertRgbaColor;
    
    gl_Position = MVP * vec4(vertPosCoords, 1.0);
}
