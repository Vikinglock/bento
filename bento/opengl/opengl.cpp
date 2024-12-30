#include "opengl.h"
#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"
#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <sstream>


enum {
    KeyStateNone,
    KeyStatePressed,
    KeyStateReleased
};

bool shouldClose;
int vertCount = 0;
int normCount = 0;
int uvCount = 0;

int buttonCount;
const unsigned char* buttons[GLFW_JOYSTICK_LAST];
int axisCount;
const float* axes[GLFW_JOYSTICK_LAST];
//opengl is so much more straightforward

// #### MAIN ####
std::string loadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
GLuint compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        std::cerr << "Shader compilation failed: " << log.data() << std::endl;
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}
GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = loadShaderSource(vertexPath);
    std::string fragmentSource = loadShaderSource(fragmentPath);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetProgramInfoLog(program, logLength, nullptr, log.data());
        std::cerr << "program fail: " << log.data() << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}


void OpenGLBento::init(const char *title, int width, int height, int x, int y){
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetWindowPos(window, x, y);
    if (!window) {
        std::cerr << "could not create glfw window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "could not initialize glad (not glad)" << std::endl;
        return;
    }

    shader = createShaderProgram("bento/shaders/shader.vs", "bento/shaders/shader.fs");
    modelLocation = glGetUniformLocation(shader, "model");
    viewLocation = glGetUniformLocation(shader, "view");
    projectionLocation = glGetUniformLocation(shader, "projection");


    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i) {
        if (glfwJoystickPresent(i)) {
            buttons[i] = glfwGetJoystickButtons(i, &buttonCount);
            axes[i] = glfwGetJoystickAxes(i, &axisCount);
        }
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &normalBuffer);
    glGenBuffers(1, &uvBuffer);

    glEnable(GL_DEPTH_TEST);
}

void OpenGLBento::focus(){
    glfwFocusWindow(window);
}

void OpenGLBento::setVerticesDirect(const std::vector<glm::vec3>& vs) {
    vertices = vs;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
}

void OpenGLBento::setNormalsDirect(const std::vector<glm::vec3>& ns) {
    normals = ns;
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
}

void OpenGLBento::setUvsDirect(const std::vector<glm::vec2>& uv) {
    uvs = uv;
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);
}

void OpenGLBento::setVertices(class vertexBuffer vs) {
    vertices = vs.getBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
}

void OpenGLBento::setNormals(class normalBuffer ns) {
    normals = ns.getBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
}

void OpenGLBento::setUvs(class uvBuffer us) {
    uvs = us.getBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);
}

void OpenGLBento::bindTexture(Texture *texture) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->getTexture());
}

void OpenGLBento::unbindTexture() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLBento::predraw() {
    glfwPollEvents();
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i) {
        if (glfwJoystickPresent(i)) {
            buttons[i] = glfwGetJoystickButtons(i, &buttonCount);
            axes[i] = glfwGetJoystickAxes(i, &axisCount);
        }
    }
    glUseProgram(shader);
}

void OpenGLBento::draw() {
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);
}

void OpenGLBento::render() {
    glfwSwapBuffers(window);
}

void OpenGLBento::setModelMatrix(const glm::mat4& m) {
    model = m;
}

void OpenGLBento::setViewMatrix(const glm::mat4& v) {
    view = v;
}

void OpenGLBento::setProjectionMatrix(const glm::mat4& p) {
    projection = p;
}

bool OpenGLBento::isRunning(){
    return !glfwWindowShouldClose(window);
}

// #### INPUT ####
void OpenGLBento::exit() {
    ImGui_ImplOpenGL3_Shutdown();
    glfwTerminate();
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader);
}


void OpenGLBento::toggleFullscreen() {
    
}
bool OpenGLBento::isWindowFocused() {
    return glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0;
}
// #### INPUT ####
bool OpenGLBento::getKey(int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}
bool OpenGLBento::getMouse(int mouse) {
    return glfwGetMouseButton(window, mouse) == GLFW_PRESS;
}

// #### MOUSE AND WINDOWS ####
glm::vec2 OpenGLBento::getWindowSize() {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    return glm::vec2(width, height);
}

glm::vec2 OpenGLBento::getWindowPos() {
    int x, y;
    glfwGetWindowPos(window, &x, &y);
    return glm::vec2(x, y);
}

void OpenGLBento::setMouseCursor(bool hide, int cursor) {
    if (hide) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

glm::vec2 OpenGLBento::getMousePosition() {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return glm::vec2(x,y)+getWindowPos();
}

void OpenGLBento::setMousePosition(glm::vec2 pos, bool needsFocus) {
    glm::vec2 windowPos = getWindowPos();
    glfwSetCursorPos(window, pos.x-windowPos.x, pos.y-windowPos.y);
    if (needsFocus) {
        glfwFocusWindow(window);
    }
}

glm::vec2 OpenGLBento::getControllerAxis(int controller, JoystickType joystick) {
    if (axes[controller] == nullptr || axisCount < 4) {
        std::cout << "error (with the controller axes (like the joysticks and stuff))" << std::endl;
        return glm::vec2(0.0f, 0.0f);
    }
    switch (joystick) {
        case GAMEPAD_JOYSTICK_LEFT:
            return glm::vec2(axes[controller][0], axes[controller][1]);
        case GAMEPAD_JOYSTICK_RIGHT:
            return glm::vec2(axes[controller][2], axes[controller][3]);
        default:
            return glm::vec2(0.0f, 0.0f);
    }
}
bool OpenGLBento::getControllerButton(int controller, ButtonType button){
    return buttons[controller][button] == GLFW_PRESS;
}

void OpenGLBento::setWindowPos(glm::vec2 pos) {
    glfwSetWindowPos(window, pos.x, pos.y);
}

glm::vec2 OpenGLBento::getDisplaySize() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return glm::vec2(width, height);
}



void OpenGLBento::initImgui() {
    IMGUI_CHECKVERSION();
    ImGuiContext* imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    });

    glfwSetCharCallback(window, [](GLFWwindow *window, unsigned int c) {
        ImGui_ImplGlfw_CharCallback(window, c);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    });

    glfwSetScrollCallback(window, [](GLFWwindow *window, double xoffset, double yoffset) {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    });

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
    });
}


void OpenGLBento::imgui() {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    ImGui_ImplOpenGL3_NewFrame();

    ImGui::NewFrame();
    static bool show_demo_window = true;
    ImGui::ShowDemoWindow(&show_demo_window);
    ImGui::Begin("Example Window");
    ImGui::Text("glfw");
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

