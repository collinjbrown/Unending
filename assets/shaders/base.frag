#version 330 core

in vec4 rgbaColor;

out vec4 color;

// The size of this array is hard-coded,
// and must be manually changed if the Renderer's
// corresponding constant is changed.

uniform sampler2D batchQuadTextures[32];

void main()
{
    color = rgbaColor;
}
