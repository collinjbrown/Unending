#include "animation.h"

#include "external/stb_image.h"

Animation::Animation(const char* file, bool alpha, int columns, int rows, float speed, std::vector<int> layout, bool loop, int filter) : width(0), height(0), internalFormat(GL_RGB), imageFormat(GL_RGB), wrapS(GL_REPEAT), wrapT(GL_REPEAT), filterMin(filter), filterMax(filter)
{
    glGenTextures(1, &this->ID);

    if (alpha)
    {
        this->internalFormat = GL_RGBA;
        this->imageFormat = GL_RGBA;
    }

    stbi_set_flip_vertically_on_load(true);

    int imageWidth;
    int imageHeight;
    int nrChannels;
    unsigned char* data = stbi_load(file, &imageWidth, &imageHeight, &nrChannels, 0);

    this->width = imageWidth;
    this->height = imageHeight;

    this->columns = columns;
    this->rows = rows;
    this->speed = speed;
    this->layout = layout;
    this->loop = loop;

    // Create
    glBindTexture(GL_TEXTURE_2D, this->ID);
    glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, imageWidth, imageHeight, 0, this->imageFormat, GL_UNSIGNED_BYTE, data);

    // Wrap & Filter Modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->filterMin);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->filterMax);

    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
}

void Animation::Bind() const
{
    glBindTexture(GL_TEXTURE_2D, this->ID);
}