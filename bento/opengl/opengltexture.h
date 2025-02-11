#ifndef OPENGLTEXTURE_H
#define OPENGLTEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class OpenGLTexture {
public:
    OpenGLTexture(const char* filepath);
    OpenGLTexture(unsigned int texture);
    ~OpenGLTexture();

    unsigned int getWidth() const { return width; }
    unsigned int getHeight() const { return height; }
    unsigned int getChannels() const { return channels; }
    unsigned int getTexture() const { return texture; }

private:
    unsigned int texture;
    int width, height, channels;
};

#endif

