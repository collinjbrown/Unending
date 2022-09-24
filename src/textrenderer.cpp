#include "textrenderer.h"
#include "game.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

glm::vec2 TextRenderer::CalculateAlignment(std::string text, float scaleX, float scaleY, TextAlignment alignment)
{
    float x = 0;
    float y = 14;

    int di = text.size();

    if (alignment == TextAlignment::left) di = 1;

    for (int i = 0; i < di; i++)
    {
        const char c = text[i];
        Character ch = characters[c];

        float w = ch.size.x * scaleX;
        float h = ch.size.y * scaleY;

        x += (ch.advance >> 6) * scaleX;
    }

    if (alignment == TextAlignment::left)
    {
        return glm::vec2(-x, -(y / 2.0f));
    }
    else if (alignment == TextAlignment::center)
    {
        return glm::vec2(-(x / 2.0f), -(y / 2.0f));
    }
    else if (alignment == TextAlignment::right)
    {
        return glm::vec2(-x, -(y / 2.0f));
    }
}

//void TextRenderer::RenderText(std::string text, glm::vec2 position, glm::vec2 size, glm::vec4 color)
//{
//    glm::vec3 
//
//    for (int i = 0; i < text.size(); i++)
//    {
//        const char c = text[i];
//        Character ch = characters[c];
//
//        float xPos = x + ch.bearing.x * scaleX;
//        float yPos = y - (ch.size.y - ch.bearing.y) * scaleY;
//
//        float w = ch.size.x * scaleX;
//        float h = ch.size.y * scaleY;
//
//        float leftX = ch.textureX / atlasWidth;
//        float rightX = (ch.textureX + (float)ch.size.x) / atlasWidth;
//        float topY = ch.size.y / (float)atlasHeight;
//
//        Quad quad;
//
//        /*quad.topLeft = { xPos + w,   yPos + h,   -100.0f,   color.r, color.g, color.b, color.a,     rightX, 0.0f, 0, 0, wMod, hMod };
//        quad.bottomLeft = { xPos + w,   yPos,       -100.0f,   color.r, color.g, color.b, color.a,     rightX, topY, 0, 0, wMod, hMod };
//        quad.bottomRight = { xPos,       yPos,       -100.0f,   color.r, color.g, color.b, color.a,     leftX,  topY, 0, 0, wMod, hMod };
//        quad.topRight = { xPos,       yPos + h,   -100.0f,   color.r, color.g, color.b, color.a,     leftX,  0.0f, 0, 0, wMod, hMod };
//
//        Game::main.renderer->prepareQuad(quad, textureAtlas, mapAtlas);*/
//
//        x += (ch.advance >> 6) * scaleX;
//    }
//
//    return;
//}
