#ifndef OPENGL_H
#define OPENGL_H

#include <iostream>
#include "../lib/imgui/imgui.h"
#include "../lib/imgui/backends/imgui_impl_glfw.h"
#include "../lib/imgui/backends/imgui_impl_opengl3.h"


#include "../features.h"

#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"
#include "../lib/glad/glad.h"
#include "../lib/GLFW/glfw3.h"
#include <vector>
#include "opengltexture.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <regex>
#include <unordered_map>

#include "../lib/AL/al.h"
#include "../lib/AL/alc.h"
#include "../sound/soundcommon.h"

class Bento;

enum{
    //  #####     KEYS     #####
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
    KEY_NUMPAD_EQUALS = GLFW_KEY_KP_EQUAL,
    KEY_NUMPAD_CLEAR = GLFW_KEY_NUM_LOCK,
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
    KEY_VOLUME_UP = -1, KEY_VOLUME_DOWN = -1, KEY_MUTE = -1,
    KEY_HELP = GLFW_KEY_INSERT, KEY_INSERT = GLFW_KEY_INSERT, KEY_HOME = GLFW_KEY_HOME,
    KEY_END = GLFW_KEY_END, KEY_PAGE_DOWN = GLFW_KEY_PAGE_DOWN, KEY_PAGE_UP = GLFW_KEY_PAGE_UP,
    KEY_UP = GLFW_KEY_UP, KEY_DOWN = GLFW_KEY_DOWN, KEY_LEFT = GLFW_KEY_LEFT, KEY_RIGHT = GLFW_KEY_RIGHT,
    KEY_FUNCTION = -1, KEY_CAPS_LOCK = GLFW_KEY_CAPS_LOCK,
    KEY_LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT, KEY_LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL, KEY_LEFT_OPTION = GLFW_KEY_LEFT_ALT, KEY_LEFT_ALT = GLFW_KEY_LEFT_ALT, KEY_LEFT_COMMAND = 0x37,
    KEY_RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT, KEY_RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL, KEY_RIGHT_OPTION = GLFW_KEY_RIGHT_ALT, KEY_RIGHT_ALT = GLFW_KEY_RIGHT_ALT, KEY_RIGHT_COMMAND = 0x36,
    //  #####     MOUSE BUTTONS     #####
    MOUSE_LEFT = GLFW_MOUSE_BUTTON_1,
    MOUSE_RIGHT = GLFW_MOUSE_BUTTON_2,
    MOUSE_MIDDLE = GLFW_MOUSE_BUTTON_3,
    MOUSE_1 = GLFW_MOUSE_BUTTON_1,
    MOUSE_2 = GLFW_MOUSE_BUTTON_2,
    MOUSE_3 = GLFW_MOUSE_BUTTON_3,
    MOUSE_4 = GLFW_MOUSE_BUTTON_4,
    MOUSE_5 = GLFW_MOUSE_BUTTON_5,
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
    Texture() : OpenGLTexture() {}
    Texture(const char* filepath) : OpenGLTexture(filepath) {}
    Texture(unsigned int filepath) : OpenGLTexture(filepath) {}
};



class Shader{
public:
    Shader(std::string vertPath, std::string fragPath);
    Shader(GLuint prgm,std::string vS,std::string fS){
        program = prgm;
        vertSource = vS;
        fragSource = fS;
    }
    ~Shader(){
        glDeleteProgram(program);
    }

    std::string getUni();

    GLuint program;
    std::unordered_map<int,GLint> textureLocs;
    std::string vertSource = "";
    std::string fragSource = "";
};



class Bento {
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
    glm::vec2 getWindowSize();
    glm::vec2 getWindowPos();
    void setWindowPos(glm::vec2 pos);
    void toggleFullscreen();
    void focus();
    bool getKey(int key);
    bool getMouse(int mouse);
    bool getKeyUp(int key);
    bool getKeyDown(int key);
    bool getMouseUp(int mouse);
    bool getMouseDown(int mouse);
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
    void enable(Feature f,bool enabled = true);

    void normalizeTexture(int index, bool normalized);

    void setUniform(std::string uniformName, float value, bool onVertex = false){glUniform1f(glGetUniformLocation(shader->program, uniformName.c_str()),value);}
    void setUniform(std::string uniformName, int value, bool onVertex = false){glUniform1i(glGetUniformLocation(shader->program, uniformName.c_str()),value);}
    void setUniform(std::string uniformName, glm::vec2 value, bool onVertex = false){glUniform2fv(glGetUniformLocation(shader->program, uniformName.c_str()),1,&value[0]);}
    void setUniform(std::string uniformName, glm::vec3 value, bool onVertex = false){glUniform3fv(glGetUniformLocation(shader->program, uniformName.c_str()),1,&value[0]);}
    void setUniform(std::string uniformName, glm::mat4 value, bool onVertex = false){glUniformMatrix4fv(glGetUniformLocation(shader->program, uniformName.c_str()), 1, GL_FALSE, &value[0][0]);}
    void setUniform(std::string uniformName, glm::vec4 value, bool onVertex = false) {glUniform4fv(glGetUniformLocation(shader->program, uniformName.c_str()), 1, &value[0]);}

    void setUniform(std::string uniformName, float* values, int count, bool onVertex = false) {glUniform1fv(glGetUniformLocation(shader->program, uniformName.c_str()), count, values);}
    void setUniform(std::string uniformName, int* values, int count, bool onVertex = false) {glUniform1iv(glGetUniformLocation(shader->program, uniformName.c_str()), count, values);}
    void setUniform(std::string uniformName, glm::vec2* values, int count, bool onVertex = false) {glUniform2fv(glGetUniformLocation(shader->program, uniformName.c_str()), count, glm::value_ptr(values[0]));}
    void setUniform(std::string uniformName, glm::vec3* values, int count, bool onVertex = false) {glUniform3fv(glGetUniformLocation(shader->program, uniformName.c_str()), count, glm::value_ptr(values[0]));}
    void setUniform(std::string uniformName, glm::vec4* values, int count, bool onVertex = false) {glUniform4fv(glGetUniformLocation(shader->program, uniformName.c_str()), count, glm::value_ptr(values[0]));}
    void setUniform(std::string uniformName, glm::mat4* values, int count, bool onVertex = false) {glUniformMatrix4fv(glGetUniformLocation(shader->program, uniformName.c_str()), count, GL_FALSE, glm::value_ptr(values[0][0]));}
    // Vector implementations
    void setUniform(std::string uniformName, const std::vector<float>& values, bool onVertex = false) {glUniform1fv(glGetUniformLocation(shader->program, uniformName.c_str()), values.size(), values.data());}
    void setUniform(std::string uniformName, const std::vector<int>& values, bool onVertex = false) {glUniform1iv(glGetUniformLocation(shader->program, uniformName.c_str()), values.size(), values.data());}
    void setUniform(std::string uniformName, const std::vector<glm::vec2>& values, bool onVertex = false) {glUniform2fv(glGetUniformLocation(shader->program, uniformName.c_str()), values.size(), glm::value_ptr(values[0]));}
    void setUniform(std::string uniformName, const std::vector<glm::vec3>& values, bool onVertex = false) {glUniform3fv(glGetUniformLocation(shader->program, uniformName.c_str()), values.size(), glm::value_ptr(values[0]));}
    void setUniform(std::string uniformName, const std::vector<glm::vec4>& values, bool onVertex = false) {glUniform4fv(glGetUniformLocation(shader->program, uniformName.c_str()), values.size(), glm::value_ptr(values[0]));}
    void setUniform(std::string uniformName, const std::vector<glm::mat4>& values, bool onVertex = false) {glUniformMatrix4fv(glGetUniformLocation(shader->program, uniformName.c_str()), values.size(), GL_FALSE, glm::value_ptr(values[0][0]));}

    void exit();

    Shader* getDefaultShader();

    //imgui

    void initImgui();
    void imguiNewFrame();

    void imguiRender();

    //lights

    void addLight(const glm::vec3 position,const glm::vec3 ambient = glm::vec3(1.0f),const glm::vec3 diffuse = glm::vec3(1.0f),const glm::vec3 specular = glm::vec3(1.0f),float constant = 1.0f,float linear = 0.09f,float quadratic = 0.032f);
    void setLightPos(int index, glm::vec3 position);
    void setLightConstants(int index, float constant);
    void setLightLinears(int index, float linear);
    void setLightQuads(int index, float quad);
    void setLightAmbients(int index, glm::vec3 ambient);
    void setLightDiffuses(int index, glm::vec3 diffuse);
    void setLightSpeculars(int index, glm::vec3 specular);

    void setAmbientColor(glm::vec3 ambient);

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

    std::string getUni();
    Shader* shader;//no, no it is not.

private:
    int startAtt, endAtt, startRT, endRT, dTInd;

    std::unordered_map<int, bool> prevKeyStates;
    std::unordered_map<int, bool> prevMouseStates;

    GLuint vao, vertexBuffer, normalBuffer, uvBuffer;//, ubo, uboIndex; me when macos (they just don't work idk why)
                                                             // i'll redo it as soon as they make windows more fun to work on (or i guess i could just use linux)
    GLuint framebuffer,depthRTexture;
    std::vector<GLuint> texture,depthTexture;
    GLuint fbo, screenTex, rbo;
    GLint tvp[4];
    float fboWidth, fboHeight;
    GLuint qvao, qvertexBuffer, quvBuffer;

    GLFWwindow* window;

    Shader* defaultShader;

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

    GLuint ambientLoc;

    glm::vec3 amb;

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    glm::vec3 pos;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;//legit it's so slow compared to metal on my mac that i gotta make it more space efficient
    std::vector<glm::vec2> uvs;//     like i love glfw but holy hell
};

#endif