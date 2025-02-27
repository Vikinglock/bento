#include "opengl.h"
#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"


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

ALCdevice* aldevice = nullptr;
ALCcontext* context = nullptr;
std::vector<ALuint> sounds;
std::vector<ALuint> buffers;

enum {
    KeyStateNone,
    KeyStatePressed,
    KeyStateReleased
};

bool shouldClose;
int vertCount = 0;
int normCount = 0;
int uvCount = 0;
int wheelX = 0;
int wheelY = 0;

glm::vec4 clearColor = glm::vec4(0.0,0.0,0.0,1.0);

int buttonCount;
const unsigned char* buttons[GLFW_JOYSTICK_LAST];
int axisCount;
const float* axes[GLFW_JOYSTICK_LAST];
//opengl is so much more straightforward

// #### MAIN ####
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
    #ifdef WINDOWS
    y+=32;//another severe case of windows sucks
    #endif
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

    defaultShader = true;

    shader = createShaderProgram("bento/shaders/shader.vs", "bento/shaders/shader.fs");
    modelLoc = glGetUniformLocation(shader, "model");
    viewLoc = glGetUniformLocation(shader, "view");
    projectionLoc = glGetUniformLocation(shader, "projection");
    positionLoc = glGetUniformLocation(shader, "tpos");


    positionsLoc = glGetUniformLocation(shader, "positions");
    constantsLoc = glGetUniformLocation(shader, "constants");
    linearsLoc = glGetUniformLocation(shader, "linears");
    quadsLoc = glGetUniformLocation(shader, "quadratics");
    ambientsLoc = glGetUniformLocation(shader, "ambients");
    diffusesLoc = glGetUniformLocation(shader, "diffuses");
    specularsLoc = glGetUniformLocation(shader, "speculars");
    numLightsLoc = glGetUniformLocation(shader, "numLights");

    ambientLoc = glGetUniformLocation(shader, "ambient");

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
    glGenFramebuffers(1, &framebuffer);
    glGenTextures(1, &colorTexture);
    glGenTextures(1, &depthTexture);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}


void OpenGLBento::initSound(){
    aldevice = alcOpenDevice(nullptr);
    context = alcCreateContext(aldevice, nullptr);
    if (!context)alcCloseDevice(aldevice);
    alcMakeContextCurrent(context);
    ALenum error = alGetError();
}


void OpenGLBento::setClearColor(glm::vec4 col){
    clearColor = col;
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
    glClearColor(clearColor.x,clearColor.y,clearColor.z,clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i) {
        if (glfwJoystickPresent(i)) {
            buttons[i] = glfwGetJoystickButtons(i, &buttonCount);
            axes[i] = glfwGetJoystickAxes(i, &axisCount);
        }
    }
    if(defaultShader)glUseProgram(shader);
}

void OpenGLBento::draw() {
    if(defaultShader){
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform3f(positionLoc,pos.x,pos.y,pos.z);

        glUniform1i(numLightsLoc,numLights);

        glUniform3fv(positionsLoc, MAX_LIGHTS, glm::value_ptr(positions[0]));
        glUniform1fv(constantsLoc, MAX_LIGHTS, constants);
        glUniform1fv(linearsLoc, MAX_LIGHTS, linears);
        glUniform1fv(quadsLoc, MAX_LIGHTS, quads);
        glUniform3fv(ambientsLoc, MAX_LIGHTS, glm::value_ptr(ambients[0]));
        glUniform3fv(diffusesLoc, MAX_LIGHTS, glm::value_ptr(diffuses[0]));
        glUniform3fv(specularsLoc, MAX_LIGHTS, glm::value_ptr(speculars[0]));

        glUniform3f(ambientLoc,amb.x,amb.y,amb.z);
    }
    
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);
}

void OpenGLBento::render() {
    glfwSwapBuffers(window);
}

void OpenGLBento::predrawTex(int width, int height) {
    glCullFace(GL_FRONT);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);


    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    glGetIntegerv(GL_VIEWPORT, tvp);
    glViewport(0, 0, width, height);
    glClearColor(clearColor.x,clearColor.y,clearColor.z,clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLBento::drawTex() {
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    float prev = projection[1][1];
    projection[1][1] = -prev;
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    projection[1][1] = prev;

    glUniform3f(positionLoc,pos.x,pos.y,pos.z);

    glUniform1i(numLightsLoc,numLights);

    glUniform3fv(positionsLoc, MAX_LIGHTS, glm::value_ptr(positions[0]));
    glUniform1fv(constantsLoc, MAX_LIGHTS, constants);
    glUniform1fv(linearsLoc, MAX_LIGHTS, linears);
    glUniform1fv(quadsLoc, MAX_LIGHTS, quads);
    glUniform3fv(ambientsLoc, MAX_LIGHTS, glm::value_ptr(ambients[0]));
    glUniform3fv(diffusesLoc, MAX_LIGHTS, glm::value_ptr(diffuses[0]));
    
    glUniform3fv(specularsLoc, MAX_LIGHTS, glm::value_ptr(speculars[0]));
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);
}

Texture* OpenGLBento::renderTex(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(tvp[0],tvp[1],tvp[2],tvp[3]);
    glCullFace(GL_BACK);
    return new Texture(colorTexture);
}

void OpenGLBento::setModelMatrix(const glm::mat4& m) {model = m;}
void OpenGLBento::setViewMatrix(const glm::mat4& v,const glm::vec3 p) {view = v;pos = p;}
void OpenGLBento::setProjectionMatrix(const glm::mat4& p) {projection = p;}
bool OpenGLBento::isRunning(){return !glfwWindowShouldClose(window);}

// #### INPUT ####
void OpenGLBento::exit() {
    ImGui_ImplOpenGL3_Shutdown();

    for(int i = 0; i < sounds.size(); i++){
        alSourceStop(sounds[i]);
        alDeleteSources(1, &sounds[i]);
        alDeleteBuffers(1, &buffers[i]);
    }

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    alcCloseDevice(aldevice);


    glfwTerminate();
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader);
    std::exit(0);
}

void OpenGLBento::setShader(Shader* shader) {
    defaultShader = false;
    currentShader = shader;
}
void OpenGLBento::resetShader() {
    defaultShader = true;
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


double OpenGLBento::getScroll(int wheel){
    if(wheel == 0)return wheelY;
    if(wheel == 1)return wheelX;
    return 0;
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


void OpenGLBento::imguiNewFrame() {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = ImVec2((float)width, (float)height);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}


    
void OpenGLBento::addLight(const glm::vec3 pos,const glm::vec3 ambient,const glm::vec3 diffuse,const glm::vec3 specular,float constant,float linear,float quadratic) {
    if (numLights >= MAX_LIGHTS) return;

    positions[numLights] = pos;
    constants[numLights] = constant;
    linears[numLights] = linear;
    quads[numLights] = quadratic;
    ambients[numLights] = ambient;
    diffuses[numLights] = diffuse;
    speculars[numLights] = specular;

    numLights++;
}

void OpenGLBento::setLightPos(int index, glm::vec3 position){positions[index] = position;}
void OpenGLBento::setLightConstants(int index, float constant){constants[index] = constant;}
void OpenGLBento::setLightLinears(int index, float linear){linears[index] = linear;}
void OpenGLBento::setLightQuads(int index, float quad){quads[index] = quad;}
void OpenGLBento::setLightAmbients(int index, glm::vec3 ambient){ambients[index] = ambient;}
void OpenGLBento::setLightDiffuses(int index, glm::vec3 diffuse){diffuses[index] = diffuse;}
void OpenGLBento::setLightSpeculars(int index, glm::vec3 specular){speculars[index] = specular;}


void OpenGLBento::setAmbientColor(glm::vec3 ambient){amb = ambient;}



void OpenGLBento::imguiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


std::string OpenGLBento::getFramework(){
    return "OpenGL";
}