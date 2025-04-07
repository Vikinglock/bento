#ifndef METAL_H
#define METAL_H

#define MAX_LIGHTS 50

#include <iostream>
#include <unordered_map>
#include <string>
#include <regex>
#include <simd/simd.h>

#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"
#include "metaltexture.h"


#include "../lib/imgui/imgui.h"
#include "../lib/imgui/backends/imgui_impl_metal.h"
#include "../lib/imgui/backends/imgui_impl_osx.h"

#include "../lib/AL/al.h"//for TWO FUNCTIONS
#include "../lib/AL/alc.h"
#include "../sound/soundcommon.h"

#include <Carbon/Carbon.h>

#ifdef __OBJC__
#import <Metal/Metal.h>
#include "metalcommon.h"
#endif

class Bento;

enum {
    //  #####     KEYS     #####
    KEY_F1 = kVK_F1, KEY_F2  = kVK_F2 , KEY_F3  = kVK_F3 , KEY_F4  = kVK_F4 ,
    KEY_F5 = kVK_F5, KEY_F6  = kVK_F6 , KEY_F7  = kVK_F7 , KEY_F8  = kVK_F8 ,
    KEY_F9 = kVK_F9, KEY_F10 = kVK_F10, KEY_F11 = kVK_F11, KEY_F12 = kVK_F12,

    KEY_F13 = kVK_F13, KEY_F14 = kVK_F14, KEY_F15 = kVK_F15, KEY_F16 = kVK_F16,
    KEY_F17 = kVK_F17, KEY_F18 = kVK_F18, KEY_F19 = kVK_F19, KEY_F20 = kVK_F20,

    KEY_NUMPAD_0 = kVK_ANSI_Keypad0,
    KEY_NUMPAD_1 = kVK_ANSI_Keypad1, KEY_NUMPAD_2 = kVK_ANSI_Keypad2, KEY_NUMPAD_3 = kVK_ANSI_Keypad3,
    KEY_NUMPAD_4 = kVK_ANSI_Keypad4, KEY_NUMPAD_5 = kVK_ANSI_Keypad5, KEY_NUMPAD_6 = kVK_ANSI_Keypad6,
    KEY_NUMPAD_7 = kVK_ANSI_Keypad7, KEY_NUMPAD_8 = kVK_ANSI_Keypad8, KEY_NUMPAD_9 = kVK_ANSI_Keypad9,

    KEY_NUM_LOCK = kVK_ANSI_KeypadClear,
    KEY_NUMPAD_ADD = kVK_ANSI_KeypadPlus,
    KEY_NUMPAD_SUBTRACT = kVK_ANSI_KeypadMinus,
    KEY_NUMPAD_MULTIPLY = kVK_ANSI_KeypadMultiply,
    KEY_NUMPAD_DIVIDE = kVK_ANSI_KeypadDivide,
    KEY_NUMPAD_DECIMAL = kVK_ANSI_KeypadDecimal,
    KEY_NUMPAD_EQUALS = kVK_ANSI_KeypadEquals,
    KEY_NUMPAD_ENTER = kVK_ANSI_KeypadEnter,
    KEY_NUMPAD_CLEAR = kVK_ANSI_KeypadClear,

    KEY_A = kVK_ANSI_A, KEY_B = kVK_ANSI_B, KEY_C = kVK_ANSI_C, KEY_D = kVK_ANSI_D, KEY_E = kVK_ANSI_E,
    KEY_F = kVK_ANSI_F, KEY_G = kVK_ANSI_G, KEY_H = kVK_ANSI_H, KEY_I = kVK_ANSI_I, KEY_J = kVK_ANSI_J,
    KEY_K = kVK_ANSI_K, KEY_L = kVK_ANSI_L, KEY_M = kVK_ANSI_M, KEY_N = kVK_ANSI_N, KEY_O = kVK_ANSI_O,
    KEY_P = kVK_ANSI_P, KEY_Q = kVK_ANSI_Q, KEY_R = kVK_ANSI_R, KEY_S = kVK_ANSI_S, KEY_T = kVK_ANSI_T,
    KEY_U = kVK_ANSI_U, KEY_V = kVK_ANSI_V, KEY_W = kVK_ANSI_W, KEY_X = kVK_ANSI_X, KEY_Y = kVK_ANSI_Y,
    KEY_Z = kVK_ANSI_Z,

    KEY_0 = kVK_ANSI_0, KEY_1 = kVK_ANSI_1, KEY_2 = kVK_ANSI_2, KEY_3 = kVK_ANSI_3, KEY_4 = kVK_ANSI_4,
    KEY_5 = kVK_ANSI_5, KEY_6 = kVK_ANSI_6, KEY_7 = kVK_ANSI_7, KEY_8 = kVK_ANSI_8, KEY_9 = kVK_ANSI_9, // beautiful

    KEY_SPACE = kVK_Space, KEY_TAB = kVK_Tab, KEY_RETURN = kVK_Return,
    KEY_DELETE = kVK_ForwardDelete, KEY_ESCAPE = kVK_Escape, KEY_BACKSPACE = kVK_Delete,

    KEY_VOLUME_UP = kVK_VolumeUp, KEY_VOLUME_DOWN = kVK_VolumeDown, KEY_MUTE = kVK_Mute,
    KEY_HELP = kVK_Help, KEY_INSERT = kVK_Help, KEY_HOME = kVK_Home,
    KEY_END = kVK_End, KEY_PAGE_DOWN = kVK_PageDown, KEY_PAGE_UP = kVK_PageUp,

    KEY_MINUS = kVK_ANSI_Minus,
    KEY_EQUALS = kVK_ANSI_Equal,
    KEY_LEFT_BRACKET = kVK_ANSI_LeftBracket,
    KEY_RIGHT_BRACKET = kVK_ANSI_RightBracket,
    KEY_BACKSLASH = kVK_ANSI_Backslash,
    KEY_SEMICOLON = kVK_ANSI_Semicolon,
    KEY_QUOTE = kVK_ANSI_Quote,
    KEY_COMMA = kVK_ANSI_Comma,
    KEY_GRAVE = kVK_ANSI_Grave,
    KEY_PERIOD = kVK_ANSI_Period,
    KEY_SLASH = kVK_ANSI_Slash,
    KEY_UP = kVK_UpArrow, KEY_DOWN = kVK_DownArrow, KEY_LEFT = kVK_LeftArrow, KEY_RIGHT = kVK_RightArrow,
    KEY_FUNCTION = kVK_Function, KEY_CAPS_LOCK = kVK_CapsLock,
    KEY_LEFT_SHIFT = kVK_Shift, KEY_LEFT_CONTROL = kVK_Control, KEY_LEFT_OPTION = kVK_Option, KEY_LEFT_ALT = kVK_Option, KEY_LEFT_COMMAND = kVK_Command,
    KEY_RIGHT_SHIFT = 0x39, KEY_RIGHT_CONTROL = 0x3C, KEY_RIGHT_OPTION = 0x3D, KEY_RIGHT_ALT = 0x3D, KEY_RIGHT_COMMAND = 0x36,
    //  #####     MOUSE BUTTONS     #####
    MOUSE_LEFT = 0x0,
    MOUSE_RIGHT = 0x1,
    MOUSE_MIDDLE = 0x2,
    MOUSE_1 = 0x0,
    MOUSE_2 = 0x1,
    MOUSE_3 = 0x2,
    MOUSE_4 = 0x3,
    MOUSE_5 = 0x4,

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
    Texture() : MetalTexture() {}
    Texture(const char* filepath) : MetalTexture(filepath) {}

    #ifdef __OBJC__
    Texture(id<MTLTexture> filepath,id<MTLSamplerState> sampler) : MetalTexture(filepath,sampler) {}
    #endif
};

class ShaderImpl;

class Shader{
private:
    ShaderImpl* impl;
public:
    Shader(const char* vertPath, const char* fragPath);

    std::string getUni(){
        std::string out;
        out.append("VERTEX : ");
        out.append("\n");
        for (const auto& [name, index] : uniformMapVert) {
            out.append(name);
            out.append(" : ");
            out.append(std::to_string(index));
            out.append(" : ");
            out.append(std::to_string(sizeMapVert[name]));
            out.append("\n");
        }
        out.append("FRAGMENT : ");
        out.append("\n");
        for (const auto& [name, index] : uniformMapFrag) {
            out.append(name);
            out.append(" : ");
            out.append(std::to_string(index));
            out.append(" : ");
            out.append(std::to_string(sizeMapFrag[name]));
            out.append("\n");
        }
        return out;
    }
    #ifdef __OBJC__
    Shader(id<MTLRenderPipelineState> plState,std::string vS,std::string fS){
        vertSource = vS;
        fragSource = fS;


        int currentOffset = 0;
        int totalSize = 0;
        std::regex inputStructRegex(R"(struct\s+main0_in\s*\{([^}]*)\};)");
        std::regex inputRegex(R"(\s*(\w+\d*)\s+(\w+)\s*\[\[.*?\]\];)");
        std::smatch inputStructMatch;
        if (std::regex_search(vS, inputStructMatch, inputStructRegex)) {
            std::string inputStructBody = inputStructMatch[1].str();
            std::sregex_iterator begin(inputStructBody.begin(), inputStructBody.end(), inputRegex);
            std::sregex_iterator end;
            for (auto it = begin; it != end; ++it) {
                std::string type = (*it)[1].str();
                std::string name = (*it)[2].str();
            }
        }
        std::regex structRegex(R"(struct\s+Uniforms\s*\{([^}]*)\};)");
        std::regex uniformRegex(R"(\s*(\w+)\s+(\w+)(\[\d+\])?;)");
        std::smatch structMatch;
        if (std::regex_search(vS, structMatch, structRegex)) {
            std::string structBody = structMatch[1].str();
            std::sregex_iterator begin(structBody.begin(), structBody.end(), uniformRegex);
            std::sregex_iterator end;
            for (auto it = begin; it != end; ++it) {
                std::string type = (*it)[1].str();
                std::string name = (*it)[2].str();
                std::string arraySize = (*it)[3].str();
                int typeSize = 0;
                if(type=="int")typeSize=sizeof(int);
                else if(type=="float")typeSize=sizeof(float);
                else if(type=="float2")typeSize=sizeof(float)*2;
                else if(type=="float3")typeSize=sizeof(float)*3;
                else if(type=="float4")typeSize=sizeof(float)*4;
                else if(type=="mat4")typeSize=sizeof(float)*16;
                else if(type=="double")typeSize=sizeof(double);
                else if(type=="double2")typeSize=sizeof(double)*2;
                else if(type=="double3")typeSize=sizeof(double)*3;
                else if(type=="double4")typeSize=sizeof(double)*4;
                int arrayCount = 1;
                if (!arraySize.empty()) {
                    std::regex arraySizeRegex(R"(\[(\d+)\])");
                    std::smatch arraySizeMatch;
                    if (std::regex_match(arraySize, arraySizeMatch, arraySizeRegex)) {
                        arrayCount = std::stoi(arraySizeMatch[1].str());
                    }
                }
                currentOffset += typeSize * arrayCount;
                uniformMapVert[name] = currentOffset;
                totalSize += typeSize * arrayCount;
            }
        }
        vertBuffer = [device newBufferWithLength:totalSize options:MTLResourceStorageModeShared];
        currentOffset = 0;
        totalSize = 0;
        if (std::regex_search(fS, structMatch, structRegex)) {
            std::string structBody = structMatch[1].str();
            std::sregex_iterator begin(structBody.begin(), structBody.end(), uniformRegex);
            std::sregex_iterator end;
            for (auto it = begin; it != end; ++it) {
                std::string type = (*it)[1].str();
                std::string name = (*it)[2].str();
                std::string arraySize = (*it)[3].str();
                int typeSize = 0;
                if(type=="int")typeSize=sizeof(int)*4;
                else if(type=="float")typeSize=sizeof(float);
                else if(type=="float2")typeSize=sizeof(float)*4;
                else if(type=="float3")typeSize=sizeof(float)*4;
                else if(type=="float4")typeSize=sizeof(float)*4;
                else if(type=="mat4")typeSize=sizeof(float)*16;
                else if(type=="double")typeSize=sizeof(double);
                else if(type=="double2")typeSize=sizeof(double)*4;
                else if(type=="double3")typeSize=sizeof(double)*4;
                else if(type=="double4")typeSize=sizeof(double)*4;
                int arrayCount = 1;
                if (!arraySize.empty()) {
                    std::regex arraySizeRegex(R"(\[(\d+)\])");
                    std::smatch arraySizeMatch;
                    if (std::regex_match(arraySize, arraySizeMatch, arraySizeRegex)) {
                        arrayCount = std::stoi(arraySizeMatch[1].str());
                    }
                }
                uniformMapFrag[name] = currentOffset;
                currentOffset += typeSize * arrayCount;
                sizeMapFrag[name] = typeSize * arrayCount;
                totalSize += typeSize * arrayCount;
            }
        }
        fragBuffer = [device newBufferWithLength:totalSize options:MTLResourceStorageModeShared];
        pipelineState = plState;
    }
    #endif

    void* pipelineState;
    void* fragBuffer;
    void* vertBuffer;
    std::string vertSource = "";
    std::string fragSource = "";
    std::unordered_map<std::string, int> uniformMapVert;
    std::unordered_map<std::string, int> sizeMapVert;
    std::unordered_map<std::string, int> uniformMapFrag;
    std::unordered_map<std::string, int> sizeMapFrag;
};

class Bento {
public:
    void init(const char *title, int w, int h, int x = 0, int y = 0);
    void initSound();
    void setClearColor(glm::vec4 col = glm::vec4(0,0,0,1));
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
    void setViewMatrix(const glm::mat4 v,const glm::vec3 p);
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
    void bindTexture(class Texture *tex,int index);
    void unbindTexture();
    void setActiveTextures(int start, int end);
    void setActiveTextures(int ind);
    void setActiveDepthTexture(int ind);
    void setActiveAttachments(int start, int end);
    void setActiveAttachments(int ind);
    void predrawTex(int width,int height);
    void drawTex();
    void renderTex();
    void renderToTex(Texture*& tex1,int ind);
    void renderDepthToTex(Texture*& tex,int ind);
    void renderToTex(Texture*& tex1, Texture*& tex2,int ind);
    void renderToTex(Texture*& tex1, Texture*& tex2, Texture*& tex3,int ind);
    void setShader(Shader* shader);

    void normalizeTexture(int index, bool normalized);

    void setUniform(std::string uniformName, glm::vec4 value, bool onvertex = false);
    void setUniform(std::string uniformName, glm::vec3 value, bool onvertex = false);
    void setUniform(std::string uniformName, float value, bool onvertex = false);
    void setUniform(std::string uniformName, int value, bool onvertex = false);
    void setUniform(std::string uniformName, glm::vec2 value, bool onvertex = false);
    void setUniform(std::string uniformName, glm::mat4 value, bool onvertex = false);
    void setUniform(std::string uniformName, std::vector<glm::vec4>& value, bool onvertex = false);
    void setUniform(std::string uniformName, std::vector<glm::vec3>& value, bool onvertex = false);
    void setUniform(std::string uniformName, std::vector<float>& value, bool onvertex = false);
    void setUniform(std::string uniformName, std::vector<int>& value, bool onvertex = false);
    void setUniform(std::string uniformName, std::vector<glm::vec2>& value, bool onvertex = false);
    void setUniform(std::string uniformName, std::vector<glm::mat4>& value, bool onvertex = false);
    void setUniform(std::string uniformName, float* values, int count, bool onVertex = false);
    void setUniform(std::string uniformName, int* values, int count, bool onVertex = false);
    void setUniform(std::string uniformName, glm::vec2* values, int count, bool onVertex = false);
    void setUniform(std::string uniformName, glm::vec3* values, int count, bool onVertex = false);
    void setUniform(std::string uniformName, glm::vec4* values, int count, bool onVertex = false);
    void setUniform(std::string uniformName, glm::mat4* values, int count, bool onVertex = false);
    
    void exit();

    Shader* getDefaultShader();

    //imgui

    void initImgui();
    void imguiNewFrame();//just realized that dear imgui handles this for me

    void imguiRender();

    //lights

    //debug

    std::string getFramework();
    std::string getOperatingSystem(){return "Macos";}

    std::string getUni();
private:
    void *rendererObjC;
};

#endif