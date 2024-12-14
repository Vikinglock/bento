#ifndef METAL_H
#define METAL_H

#include <iostream>
#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"
#include "lib/glm/gtc/type_ptr.hpp"
#include "metaltexture.h"

#ifdef __OBJC__
#import <Metal/Metal.h>
#include "metalcommon.h"
#endif

enum{
    //  #####     KEYS     #####
    KEY_UNKNOWN = -1,
    KEY_F1 = 0x7A, KEY_F2 = 0x78, KEY_F3 = 0x63, KEY_F4 = 0x76,
    KEY_F5 = 0x60, KEY_F6 = 0x61, KEY_F7 = 0x62, KEY_F8 = 0x64,
    KEY_F9 = 0x65, KEY_F10 = 0x6D, KEY_F11 = 0x67, KEY_F12 = 0x6F,
    KEY_NUM_LOCK = 0x45, KEY_NUMPAD_0 = 0x52, KEY_NUMPAD_1 = 0x53,
    KEY_NUMPAD_2 = 0x54, KEY_NUMPAD_3 = 0x55, KEY_NUMPAD_4 = 0x56,
    KEY_NUMPAD_5 = 0x57, KEY_NUMPAD_6 = 0x58, KEY_NUMPAD_7 = 0x59,
    KEY_NUMPAD_8 = 0x5B, KEY_NUMPAD_9 = 0x5C, KEY_NUMPAD_ADD = 0x45,
    KEY_NUMPAD_SUBTRACT = 0x4E, KEY_NUMPAD_MULTIPLY = 0x43,
    KEY_NUMPAD_DIVIDE = 0x4B,
    KEY_A = 0x00, KEY_B = 0x0B, KEY_C = 0x08, KEY_D = 0x02,
    KEY_E = 0x0E, KEY_F = 0x03, KEY_G = 0x05, KEY_H = 0x04,
    KEY_I = 0x22, KEY_J = 0x26, KEY_K = 0x28, KEY_L = 0x25,
    KEY_M = 0x2E, KEY_N = 0x2D, KEY_O = 0x1F, KEY_P = 0x23,
    KEY_Q = 0x0C, KEY_R = 0x0F, KEY_S = 0x01, KEY_T = 0x11,
    KEY_U = 0x20, KEY_V = 0x09, KEY_W = 0x0D, KEY_X = 0x07,
    KEY_Y = 0x10, KEY_Z = 0x06,
    KEY_0 = 0x1D, KEY_1 = 0x12, KEY_2 = 0x13, KEY_3 = 0x14,
    KEY_4 = 0x15, KEY_5 = 0x17, KEY_6 = 0x16, KEY_7 = 0x1A,
    KEY_8 = 0x1C, KEY_9 = 0x19,
    KEY_SPACE = 0x31, KEY_TAB = 0x30, KEY_RETURN = 0x24,
    KEY_DELETE = 0x33, KEY_ESCAPE = 0x35, KEY_BACKSPACE = 0x33,
    KEY_EXCLAMATION = 0x12,
    KEY_AT = 0x13,
    KEY_HASH = 0x14,
    KEY_DOLLAR = 0x15,
    KEY_PERCENT = 0x17,
    KEY_CARET = 0x16,
    KEY_AMPERSAND = 0x1A,
    KEY_ASTERISK = 0x1C,
    KEY_LEFT_PARENTHESIS = 0x19,
    KEY_RIGHT_PARENTHESIS = 0x1D,
    KEY_MINUS = 0x1B,
    KEY_EQUALS = 0x18,
    KEY_LEFT_BRACKET = 0x1E,
    KEY_RIGHT_BRACKET = 0x20,
    KEY_BACKSLASH = 0x21,
    KEY_SEMICOLON = 0x29,
    KEY_QUOTE = 0x27,
    KEY_COMMA = 0x2C,
    KEY_PERIOD = 0x2E,
    KEY_SLASH = 0x2F,
    KEY_UP = 0x7E, KEY_DOWN = 0x7D, KEY_LEFT = 0x7B, KEY_RIGHT = 0x7C,
    KEY_LEFT_SHIFT = 0x38, KEY_LEFT_CONTROL = 0x3B, KEY_LEFT_OPTION = 0x3A, KEY_LEFT_ALT = 0x3A, KEY_LEFT_COMMAND = 0x37,
    KEY_RIGHT_SHIFT = 0x39, KEY_RIGHT_CONTROL = 0x3C, KEY_RIGHT_OPTION = 0x3D, KEY_RIGHT_ALT = 0x3D, KEY_RIGHT_COMMAND = 0x36,
    KEY_VOLUME_UP = 0x48, KEY_VOLUME_DOWN = 0x49, KEY_MUTE = 0x4A,
    //  #####     MOUSE BUTTONS     #####
    KEY_MOUSE_LEFT = 0x50,
    KEY_MOUSE_RIGHT = 0x51,
    KEY_MOUSE_MIDDLE = 0x52,
    KEY_MOUSE_X1 = 0x53,
    KEY_MOUSE_X2 = 0x54,

    //  #####     OTHER     #####
};


class Texture : public MetalTexture {
public:
    Texture(const char* filepath) : MetalTexture(filepath) {}
};

class MetalBento {
public:
    void init(const char *title, int w, int h);
    void predraw();
    void draw();
    void render();
    bool isRunning();
    void setVertices(const std::vector<glm::vec3>& vertices);
    void setNormals(const std::vector<glm::vec3>& normals);
    void setUvs(const std::vector<glm::vec2>& uvs);
    void setModelMatrix(const glm::mat4& m);
    void setViewMatrix(const glm::mat4& v);
    void setProjectionMatrix(const glm::mat4& p);
    glm::vec2 getWindowSize();
    glm::vec2 getWindowPos();
    void setWindowPos(glm::vec2 pos);
    void toggleFullscreen();
    bool getKey(int key);
    void setMouseCursor(bool hide, int cursor);
    void setMousePosition(glm::vec2 pos, bool needsFocus = false);
    glm::vec2 getMousePosition();
    bool isWindowFocused();
    glm::vec2 getDisplaySize();
    void bindTexture(Texture *tex, int slot);
    void unbindTexture();
    void exit();

private:
    void *rendererObjC;
};

#endif