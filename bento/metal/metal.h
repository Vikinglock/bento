#ifndef METAL_H
#define METAL_H

#include <iostream>
#include <unordered_map>
#include <string>

#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"
#include "metaltexture.h"


#include "../lib/imgui/imgui.h"
#include "../lib/imgui/backends/imgui_impl_metal.h"
#include "../lib/imgui/backends/imgui_impl_osx.h"


#ifdef __OBJC__
#import <Metal/Metal.h>
#include "metalcommon.h"
#endif

enum {
    //  #####     KEYS     #####
    KEY_UNKNOWN = -1,
    KEY_F1 = 0x7A, KEY_F2 = 0x78, KEY_F3 = 0x63, KEY_F4 = 0x76,
    KEY_F5 = 0x60, KEY_F6 = 0x61, KEY_F7 = 0x62, KEY_F8 = 0x64,
    KEY_F9 = 0x65, KEY_F10 = 0x6D, KEY_F11 = 0x67, KEY_F12 = 0x6F,

    KEY_NUMPAD_0 = 0x52,
    KEY_NUMPAD_1 = 0x53, KEY_NUMPAD_2 = 0x54, KEY_NUMPAD_3 = 0x55,
    KEY_NUMPAD_4 = 0x56, KEY_NUMPAD_5 = 0x57, KEY_NUMPAD_6 = 0x58,
    KEY_NUMPAD_7 = 0x59, KEY_NUMPAD_8 = 0x5B, KEY_NUMPAD_9 = 0x5C,

    KEY_NUM_LOCK = 0x47,
    KEY_NUMPAD_ADD = 0x45,
    KEY_NUMPAD_SUBTRACT = 0x4E,
    KEY_NUMPAD_MULTIPLY = 0x43,
    KEY_NUMPAD_DIVIDE = 0x4B,
    KEY_NUMPAD_DECIMAL = 0x41,
    KEY_NUMPAD_ENTER = 0x4C,

    KEY_A = 0x00, KEY_B = 0x0B, KEY_C = 0x08, KEY_D = 0x02, KEY_E = 0x0E,
    KEY_F = 0x03, KEY_G = 0x05, KEY_H = 0x04, KEY_I = 0x22, KEY_J = 0x26,
    KEY_K = 0x28, KEY_L = 0x25, KEY_M = 0x2E, KEY_N = 0x2D, KEY_O = 0x1F,
    KEY_P = 0x23, KEY_Q = 0x0C, KEY_R = 0x0F, KEY_S = 0x01, KEY_T = 0x11,
    KEY_U = 0x20, KEY_V = 0x09, KEY_W = 0x0D, KEY_X = 0x07, KEY_Y = 0x10,
    KEY_Z = 0x06,

    KEY_0 = 0x1D, KEY_1 = 0x12, KEY_2 = 0x13, KEY_3 = 0x14, KEY_4 = 0x15,
    KEY_5 = 0x17, KEY_6 = 0x16, KEY_7 = 0x1A, KEY_8 = 0x1C, KEY_9 = 0x19, // beautiful

    KEY_SPACE = 0x31, KEY_TAB = 0x30, KEY_RETURN = 0x24,
    KEY_DELETE = 0x75, KEY_ESCAPE = 0x35, KEY_BACKSPACE = 0x33,

    KEY_MINUS = 0x1B,
    KEY_EQUALS = 0x18,
    KEY_LEFT_BRACKET = 0x1E,
    KEY_RIGHT_BRACKET = 0x20,
    KEY_BACKSLASH = 0x21,
    KEY_SEMICOLON = 0x29,
    KEY_QUOTE = 0x27,
    KEY_COMMA = 0x2B,
    KEY_GRAVE = 0x32,
    KEY_PERIOD = 0x2F,
    KEY_SLASH = 0x2C,
    KEY_UP = 0x7E, KEY_DOWN = 0x7D, KEY_LEFT = 0x7B, KEY_RIGHT = 0x7C,
    KEY_LEFT_SHIFT = 0x38, KEY_LEFT_CONTROL = 0x3B, KEY_LEFT_OPTION = 0x3A, KEY_LEFT_ALT = 0x3A, KEY_LEFT_COMMAND = 0x37,
    KEY_RIGHT_SHIFT = 0x39, KEY_RIGHT_CONTROL = 0x3C, KEY_RIGHT_OPTION = 0x3D, KEY_RIGHT_ALT = 0x3D, KEY_RIGHT_COMMAND = 0x36,
    //  #####     MOUSE BUTTONS     #####
    MOUSE_LEFT = 0x0,
    MOUSE_RIGHT = 0x1,
    MOUSE_MIDDLE = 0x2,
    MOUSE_X1 = 0x3,
    MOUSE_X2 = 0x4,

    //  #####     OTHER     #####
};
//because dearimgui doesn't give a keycallback for metal?????????
static std::unordered_map<int, ImGuiKey> KeytoDearImguiKey = {
    {KEY_F1, ImGuiKey_F1}, {KEY_F2, ImGuiKey_F2}, {KEY_F3, ImGuiKey_F3}, {KEY_F4, ImGuiKey_F4},
    {KEY_F5, ImGuiKey_F5}, {KEY_F6, ImGuiKey_F6}, {KEY_F7, ImGuiKey_F7}, {KEY_F8, ImGuiKey_F8},
    {KEY_F9, ImGuiKey_F9}, {KEY_F10, ImGuiKey_F10}, {KEY_F11, ImGuiKey_F11}, {KEY_F12, ImGuiKey_F12},


    {KEY_NUMPAD_0, ImGuiKey_Keypad0},
    {KEY_NUMPAD_1, ImGuiKey_Keypad1},{KEY_NUMPAD_2, ImGuiKey_Keypad2},{KEY_NUMPAD_3, ImGuiKey_Keypad3},
    {KEY_NUMPAD_4, ImGuiKey_Keypad4},{KEY_NUMPAD_5, ImGuiKey_Keypad5},{KEY_NUMPAD_6, ImGuiKey_Keypad6},
    {KEY_NUMPAD_7, ImGuiKey_Keypad7},{KEY_NUMPAD_8, ImGuiKey_Keypad8},{KEY_NUMPAD_9, ImGuiKey_Keypad9},

    {KEY_NUM_LOCK, ImGuiKey_NumLock},
    {KEY_NUMPAD_ADD, ImGuiKey_KeypadAdd},
    {KEY_NUMPAD_SUBTRACT, ImGuiKey_KeypadSubtract},
    {KEY_NUMPAD_MULTIPLY, ImGuiKey_KeypadMultiply},
    {KEY_NUMPAD_DIVIDE, ImGuiKey_KeypadDivide},
    {KEY_NUMPAD_DECIMAL, ImGuiKey_KeypadDecimal},
    {KEY_NUMPAD_ENTER, ImGuiKey_KeypadEnter},

    {KEY_A, ImGuiKey_A}, {KEY_B, ImGuiKey_B}, {KEY_C, ImGuiKey_C}, {KEY_D, ImGuiKey_D},
    {KEY_E, ImGuiKey_E}, {KEY_F, ImGuiKey_F}, {KEY_G, ImGuiKey_G}, {KEY_H, ImGuiKey_H},
    {KEY_I, ImGuiKey_I}, {KEY_J, ImGuiKey_J}, {KEY_K, ImGuiKey_K}, {KEY_L, ImGuiKey_L},
    {KEY_M, ImGuiKey_M}, {KEY_N, ImGuiKey_N}, {KEY_O, ImGuiKey_O}, {KEY_P, ImGuiKey_P},
    {KEY_Q, ImGuiKey_Q}, {KEY_R, ImGuiKey_R}, {KEY_S, ImGuiKey_S}, {KEY_T, ImGuiKey_T},
    {KEY_U, ImGuiKey_U}, {KEY_V, ImGuiKey_V}, {KEY_W, ImGuiKey_W}, {KEY_X, ImGuiKey_X},
    {KEY_Y, ImGuiKey_Y}, {KEY_Z, ImGuiKey_Z},

    {KEY_0, ImGuiKey_0}, {KEY_1, ImGuiKey_1}, {KEY_2, ImGuiKey_2}, {KEY_3, ImGuiKey_3},
    {KEY_4, ImGuiKey_4}, {KEY_5, ImGuiKey_5}, {KEY_6, ImGuiKey_6}, {KEY_7, ImGuiKey_7},
    {KEY_8, ImGuiKey_8}, {KEY_9, ImGuiKey_9},

    {KEY_SPACE, ImGuiKey_Space}, {KEY_TAB, ImGuiKey_Tab}, {KEY_RETURN, ImGuiKey_Enter},
    {KEY_DELETE, ImGuiKey_Delete}, {KEY_BACKSPACE, ImGuiKey_Backspace}, {KEY_ESCAPE, ImGuiKey_Escape},

    {KEY_UP, ImGuiKey_UpArrow}, {KEY_DOWN, ImGuiKey_DownArrow}, {KEY_LEFT, ImGuiKey_LeftArrow}, {KEY_RIGHT, ImGuiKey_RightArrow},

    {KEY_LEFT_SHIFT, ImGuiKey_LeftShift}, {KEY_RIGHT_SHIFT, ImGuiKey_RightShift},
    {KEY_LEFT_CONTROL, ImGuiKey_LeftCtrl}, {KEY_RIGHT_CONTROL, ImGuiKey_RightCtrl},
    {KEY_LEFT_OPTION, ImGuiKey_LeftAlt}, {KEY_RIGHT_OPTION, ImGuiKey_RightAlt},
    {KEY_LEFT_ALT, ImGuiKey_LeftAlt}, {KEY_RIGHT_ALT, ImGuiKey_RightAlt},
    {KEY_LEFT_COMMAND, ImGuiKey_LeftSuper}, {KEY_RIGHT_COMMAND, ImGuiKey_RightSuper},

    {KEY_MINUS, ImGuiKey_Minus}, {KEY_EQUALS, ImGuiKey_Equal},
    {KEY_LEFT_BRACKET, ImGuiKey_LeftBracket}, {KEY_RIGHT_BRACKET, ImGuiKey_RightBracket},
    {KEY_BACKSLASH, ImGuiKey_Backslash}, {KEY_SEMICOLON, ImGuiKey_Semicolon}, {KEY_QUOTE, ImGuiKey_Apostrophe},
    {KEY_COMMA, ImGuiKey_Comma},
    {KEY_GRAVE, ImGuiKey_GraveAccent},
    {KEY_PERIOD, ImGuiKey_Period},
    {KEY_SLASH, ImGuiKey_Slash},
};
enum JoystickType{
    GAMEPAD_JOYSTICK_LEFT,
    GAMEPAD_JOYSTICK_RIGHT,
};
enum ButtonType{
    GAMEPAD_KEY_A,
    GAMEPAD_KEY_B,
    GAMEPAD_KEY_X,
    GAMEPAD_KEY_Y,

    GAMEPAD_KEY_R,
    GAMEPAD_KEY_R2,
    GAMEPAD_KEY_R3,

    GAMEPAD_KEY_L,
    GAMEPAD_KEY_L2,
    GAMEPAD_KEY_L3,

    GAMEPAD_KEY_START,
    GAMEPAD_KEY_SELECT,
    GAMEPAD_KEY_HOME,
    GAMEPAD_KEY_SCREENSHOT,

    GAMEPAD_KEY_UP,
    GAMEPAD_KEY_DOWN,
    GAMEPAD_KEY_LEFT,
    GAMEPAD_KEY_RIGHT,
};


class vertexBuffer {
public:
    void setBuffer(const std::vector<glm::vec3>& buf);
    int size(){return count;}
    void* getBuffer();
private:
    void* buffer;
    int count;
};

class normalBuffer {
public:
    void setBuffer(const std::vector<glm::vec3>& buf);
    int size(){return count;}
    void* getBuffer();
private:
    void* buffer;
    int count;
};

class uvBuffer {
public:
    void setBuffer(const std::vector<glm::vec2>& buf);
    int size(){return count;}
    void* getBuffer();
private:
    void* buffer;
    int count;
};

class Texture : public MetalTexture {
public:
    Texture(const char* filepath) : MetalTexture(filepath) {}
};

class MetalBento {
public:
    void init(const char *title, int w, int h, int x = 0, int y = 0);
    void predraw();
    void draw();
    void render();
    bool isRunning();
    void setVerticesDirect(const std::vector<glm::vec3>& vertices);
    void setNormalsDirect(const std::vector<glm::vec3>& normals);
    void setUvsDirect(const std::vector<glm::vec2>& uvs);
    void setVertices(vertexBuffer vertices);
    void setNormals(normalBuffer normals);
    void setUvs(uvBuffer uvs);
    void setModelMatrix(const glm::mat4 m);
    void setViewMatrix(const glm::mat4 v);
    void setProjectionMatrix(const glm::mat4 p);
    glm::vec2 getWindowSize();
    glm::vec2 getWindowPos();
    void setWindowPos(glm::vec2 pos);
    void toggleFullscreen();
    void focus();
    bool getKey(int key);
    bool getMouse(int mouse);
    double getScroll(int wheel);
    void setMouseCursor(bool hide, int cursor);
    void setMousePosition(glm::vec2 pos, bool needsFocus = false);
    glm::vec2 getMousePosition();
    glm::vec2 getControllerAxis(int controller, JoystickType joystick);
    bool getControllerButton(int controller, ButtonType button);
    bool isWindowFocused();
    glm::vec2 getDisplaySize();
    void bindTexture(class Texture *tex);
    void unbindTexture();
    void exit();

    //imgui

    void initImgui();
    void imguiNewFrame();//just realized that dear imgui handles this for me

    void imguiRender();

    //debug

    std::string getFramework();

private:
    void *rendererObjC;
};

#endif