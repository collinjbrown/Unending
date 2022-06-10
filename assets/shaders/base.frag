#version 330 core

in vec4 rgbaColor;
in vec2 texCoords;
in float texIndex;

out vec4 color;

uniform sampler2D batchTextures[32];

void main()
{
    int index = int(texIndex);
    color = rgbaColor * texture(batchTextures[index], texCoords);
}
