#include "opengltexture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../lib/stb_image.h"

OpenGLTexture::OpenGLTexture(const char* filepath) {
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(filepath, &width, &height, &channels, STBI_rgb_alpha);
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);
}
OpenGLTexture::OpenGLTexture() {}

OpenGLTexture::OpenGLTexture(unsigned int tex) {
    texture = tex;
}

OpenGLTexture::~OpenGLTexture() {
    glDeleteTextures(1, &texture);
}