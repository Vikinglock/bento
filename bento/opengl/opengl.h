#ifndef OPENGL_H
#define OPENGL_H

#define MAX_LIGHTS 50

#include <iostream>

#include "../lib/imgui/imgui.h"
#include "../lib/imgui/backends/imgui_impl_glfw.h"
#include "../lib/imgui/backends/imgui_impl_opengl3.h"


#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"
#include "../lib/glad/glad.h"
#include "../lib/GLFW/glfw3.h"
#include <vector>
#include "opengltexture.h"


#include "../lib/AL/al.h"//gonna integrate this later
#include "../lib/AL/alc.h"//yes i copy pasted it
#include "../sound/soundcommon.h"


enum{
    //  #####     KEYS     #####
    KEY_UNKNOWN = -1,
    KEY_F1 = GLFW_KEY_F1, KEY_F2 = GLFW_KEY_F2, KEY_F3 = GLFW_KEY_F3, KEY_F4 = GLFW_KEY_F4,
    KEY_F5 = GLFW_KEY_F5, KEY_F6 = GLFW_KEY_F6, KEY_F7 = GLFW_KEY_F7, KEY_F8 = GLFW_KEY_F8,
    KEY_F9 = GLFW_KEY_F9, KEY_F10 = GLFW_KEY_F10, KEY_F11 = GLFW_KEY_F11, KEY_F12 = GLFW_KEY_F12,
    KEY_NUM_LOCK = GLFW_KEY_NUM_LOCK, KEY_NUMPAD_0 = GLFW_KEY_KP_0, KEY_NUMPAD_1 = GLFW_KEY_KP_0,
    KEY_NUMPAD_2 = GLFW_KEY_KP_2, KEY_NUMPAD_3 = GLFW_KEY_KP_3, KEY_NUMPAD_4 = GLFW_KEY_KP_4,
    KEY_NUMPAD_5 = GLFW_KEY_KP_5, KEY_NUMPAD_6 = GLFW_KEY_KP_6, KEY_NUMPAD_7 = GLFW_KEY_KP_7,
    KEY_NUMPAD_8 = GLFW_KEY_KP_8, KEY_NUMPAD_9 = GLFW_KEY_KP_9, KEY_NUMPAD_ADD = GLFW_KEY_KP_ADD,
    KEY_NUMPAD_SUBTRACT = GLFW_KEY_KP_SUBTRACT, KEY_NUMPAD_MULTIPLY = GLFW_KEY_KP_MULTIPLY,
    KEY_NUMPAD_DIVIDE = GLFW_KEY_KP_DIVIDE, KEY_NUMPAD_ENTER = GLFW_KEY_KP_ENTER,
    KEY_NUMPAD_DECIMAL = GLFW_KEY_KP_DECIMAL,
    KEY_A = GLFW_KEY_A, KEY_B = GLFW_KEY_B, KEY_C = GLFW_KEY_C, KEY_D = GLFW_KEY_D,
    KEY_E = GLFW_KEY_E, KEY_F = GLFW_KEY_F, KEY_G = GLFW_KEY_G, KEY_H = GLFW_KEY_H,
    KEY_I = GLFW_KEY_I, KEY_J = GLFW_KEY_J, KEY_K = GLFW_KEY_K, KEY_L = GLFW_KEY_L,
    KEY_M = GLFW_KEY_M, KEY_N = GLFW_KEY_N, KEY_O = GLFW_KEY_O, KEY_P = GLFW_KEY_P,
    KEY_Q = GLFW_KEY_Q, KEY_R = GLFW_KEY_R, KEY_S = GLFW_KEY_S, KEY_T = GLFW_KEY_T,
    KEY_U = GLFW_KEY_U, KEY_V = GLFW_KEY_V, KEY_W = GLFW_KEY_W, KEY_X = GLFW_KEY_X,
    KEY_Y = GLFW_KEY_Y, KEY_Z = GLFW_KEY_Z,
    KEY_0 = GLFW_KEY_0, KEY_1 = GLFW_KEY_1, KEY_2 = GLFW_KEY_2, KEY_3 = GLFW_KEY_3,
    KEY_4 = GLFW_KEY_4, KEY_5 = GLFW_KEY_5, KEY_6 = GLFW_KEY_6, KEY_7 = GLFW_KEY_7,
    KEY_8 = GLFW_KEY_8, KEY_9 = GLFW_KEY_9,
    KEY_SPACE = GLFW_KEY_SPACE, KEY_TAB = GLFW_KEY_TAB, KEY_RETURN = GLFW_KEY_ENTER,
    KEY_DELETE = GLFW_KEY_DELETE, KEY_ESCAPE = GLFW_KEY_ESCAPE, KEY_BACKSPACE = GLFW_KEY_BACKSPACE,
    KEY_MINUS = GLFW_KEY_MINUS,
    KEY_EQUALS = GLFW_KEY_EQUAL,
    KEY_LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET,
    KEY_RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET,
    KEY_BACKSLASH = GLFW_KEY_BACKSLASH,
    KEY_SEMICOLON = GLFW_KEY_SEMICOLON,
    KEY_QUOTE = GLFW_KEY_APOSTROPHE,
    KEY_COMMA = GLFW_KEY_COMMA,
    KEY_GRAVE = GLFW_KEY_GRAVE_ACCENT,
    KEY_PERIOD = GLFW_KEY_PERIOD,
    KEY_SLASH = GLFW_KEY_SLASH,
    KEY_UP = GLFW_KEY_UP, KEY_DOWN = GLFW_KEY_DOWN, KEY_LEFT = GLFW_KEY_LEFT, KEY_RIGHT = GLFW_KEY_RIGHT,
    KEY_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT, KEY_LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL, KEY_LEFT_OPTION = GLFW_KEY_LEFT_ALT, KEY_LEFT_ALT = GLFW_KEY_LEFT_ALT, KEY_LEFT_COMMAND = 0x37,
    KEY_RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT, KEY_RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL, KEY_RIGHT_OPTION = GLFW_KEY_RIGHT_ALT, KEY_RIGHT_ALT = GLFW_KEY_RIGHT_ALT, KEY_RIGHT_COMMAND = 0x36,
    //  #####     MOUSE BUTTONS     #####
    MOUSE_LEFT = GLFW_MOUSE_BUTTON_1,
    MOUSE_RIGHT = GLFW_MOUSE_BUTTON_2,
    MOUSE_MIDDLE = GLFW_MOUSE_BUTTON_3,
    MOUSE_X1 = GLFW_MOUSE_BUTTON_4,
    MOUSE_X2 = GLFW_MOUSE_BUTTON_5,

    //  #####     OTHER     #####
};

enum JoystickType{
    GAMEPAD_JOYSTICK_LEFT,
    GAMEPAD_JOYSTICK_RIGHT,
};
enum ButtonType{
    GAMEPAD_KEY_A = GLFW_GAMEPAD_BUTTON_A,
    GAMEPAD_KEY_B = GLFW_GAMEPAD_BUTTON_B,
    GAMEPAD_KEY_X = GLFW_GAMEPAD_BUTTON_X,
    GAMEPAD_KEY_Y = GLFW_GAMEPAD_BUTTON_Y,

    GAMEPAD_KEY_R1 = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
    GAMEPAD_KEY_R2 = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER+2,
    GAMEPAD_KEY_R3 = GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,

    GAMEPAD_KEY_L1 = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,
    GAMEPAD_KEY_L2 = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER+2,
    GAMEPAD_KEY_L3 = GLFW_GAMEPAD_BUTTON_LEFT_THUMB,

    GAMEPAD_KEY_START = GLFW_GAMEPAD_BUTTON_START,
    GAMEPAD_KEY_SELECT = GLFW_GAMEPAD_BUTTON_BACK,
    GAMEPAD_KEY_HOME = GLFW_GAMEPAD_BUTTON_GUIDE,
    GAMEPAD_KEY_SCREENSHOT = GLFW_GAMEPAD_BUTTON_GUIDE,

    GAMEPAD_KEY_UP = GLFW_GAMEPAD_BUTTON_DPAD_UP,
    GAMEPAD_KEY_DOWN = GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
    GAMEPAD_KEY_LEFT = GLFW_GAMEPAD_BUTTON_DPAD_LEFT,
    GAMEPAD_KEY_RIGHT = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
};

class vertexBuffer {
public:
    void setBuffer(const std::vector<glm::vec3>& buf){
        buffer = buf;
        count = buf.size()/3;
    }
    int size(){return count;}
    std::vector<glm::vec3> getBuffer(){
        return buffer;
    }
private:
    std::vector<glm::vec3> buffer;
    int count;
};
class normalBuffer {
public:
    void setBuffer(const std::vector<glm::vec3>& buf){
        buffer = buf;
        count = buf.size()/3;
    }
    int size(){return count;}
    std::vector<glm::vec3> getBuffer(){
        return buffer;
    }
private:
    std::vector<glm::vec3> buffer;
    int count;
};
class uvBuffer {
public:
    void setBuffer(const std::vector<glm::vec2>& buf){
        buffer = buf;
        count = buf.size()/2;
    }
    int size(){return count;}
    std::vector<glm::vec2> getBuffer(){
        return buffer;
    }
private:
    std::vector<glm::vec2> buffer;
    int count;
};

class Texture : public OpenGLTexture {
public:
    Texture(const char* filepath) : OpenGLTexture(filepath) {}
    Texture(unsigned int filepath) : OpenGLTexture(filepath) {}
};

class OpenGLBento {
public:
    void init(const char *title, int width, int height, int x = 0, int y = 0);
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
    void setProjectionMatrix(const glm::mat4& m);
    void setViewMatrix(const glm::mat4& v,const glm::vec3 p);
    void setModelMatrix(const glm::mat4& p);
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
    void predrawTex(int width,int height);
    void drawTex();
    Texture* renderTex();
    void exit();

    //imgui

    void initImgui();
    void imguiNewFrame();

    void imguiRender();

    //lights

    void addLight(const glm::vec3& position,const glm::vec3& ambient = glm::vec3(1.0f),const glm::vec3& diffuse = glm::vec3(1.0f),const glm::vec3& specular = glm::vec3(1.0f),float constant = 1.0f,float linear = 0.09f,float quadratic = 0.032f);
    void setLightPos(int index, glm::vec3& position);
    void setLightConstants(int index, float constant);
    void setLightLinears(int index, float linear);
    void setLightQuads(int index, float quad);
    void setLightAmbients(int index, glm::vec3& ambient);
    void setLightDiffuses(int index, glm::vec3& diffuse);
    void setLightSpeculars(int index, glm::vec3& specular);

    //debug

    std::string getFramework();
    std::string getOperatingSystem(){
        #ifdef WINDOWS
            return "Windows";
        #elif MACOS
            return "Macos";
        #elif LINUX
            return "Linux";
        #endif
    }

private:
    int numLights;
    glm::vec3 positions[MAX_LIGHTS];
    float constants[MAX_LIGHTS];
    float linears[MAX_LIGHTS];
    float quads[MAX_LIGHTS];
    glm::vec3 ambients[MAX_LIGHTS];
    glm::vec3 diffuses[MAX_LIGHTS];
    glm::vec3 speculars[MAX_LIGHTS];


    GLuint vao, vertexBuffer, normalBuffer, uvBuffer;//, ubo, uboIndex; me when macos (they just don't work idk why)
                                                             // i'll redo it as soon as they make windows more fun to work on (or i guess i could just use linux)
    GLuint framebuffer, colorTexture, depthTexture;
    GLuint fbo, screenTex, rbo;
    GLint tvp[4];
    float fboWidth, fboHeight;
    GLuint qvao, qvertexBuffer, quvBuffer;

    GLuint shader, postShader;//is the entire shader stored in the uint because that'd be pretty crazy

    GLFWwindow* window;

    GLuint modelLoc;
    GLuint viewLoc;
    GLuint projectionLoc;
    GLuint positionLoc;

    GLuint positionsLoc;
    GLuint constantsLoc;
    GLuint linearsLoc;
    GLuint quadsLoc;
    GLuint ambientsLoc;
    GLuint diffusesLoc;
    GLuint specularsLoc;
    GLuint numLightsLoc;

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    glm::vec3 pos;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;//legit it's so slow compared to metal on my mac that i gotta make it more space efficient
    std::vector<glm::vec2> uvs;//     like i love glfw but holy hell
};

#endif