#include "opengltexture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

OpenGLTexture::OpenGLTexture(const char* filepath) {
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);
}

OpenGLTexture::~OpenGLTexture() {
    glDeleteTextures(1, &texture);
}