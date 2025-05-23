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
GLuint createShaderProgram(const std::string& vertexSource, const std::string& fragmentSource) {
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
bool useDefShader = true;

std::vector<bool> normalizedTextures;

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

std::string vertShaderSource="",fragShaderSource="";

glm::vec4 clearColor = glm::vec4(0.0,0.0,0.0,1.0);

int buttonCount;
const unsigned char* buttons[GLFW_JOYSTICK_LAST];
int axisCount;
const float* axes[GLFW_JOYSTICK_LAST];
//opengl is so much more straightforward
//metal has grown on me though

// #### MAIN ####
void Bento::init(const char *title, int width, int height, int x, int y){
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

    vertShaderSource = loadShaderSource("./bento/shaders/shader.vs");
    fragShaderSource = loadShaderSource("./bento/shaders/shader.fs");

    size_t pos = vertShaderSource.rfind("gl_Position");
    if (pos != std::string::npos) {
        size_t endPos = vertShaderSource.find(";", pos);
        if (endPos != std::string::npos) {
            vertShaderSource.insert(endPos + 1, "\ngl_Position.y = -gl_Position.y;");
        }
    }
    

    GLuint shd = createShaderProgram(vertShaderSource,fragShaderSource);
    defaultShader = new Shader(shd,vertShaderSource,fragShaderSource);

    std::smatch match;
    auto begin = fragShaderSource.cbegin();
    auto end = fragShaderSource.cend();
    std::regex textureFindThingRegex(R"((sampler\w+)\s+(\w+);)");
    int index = 0;
    while (std::regex_search(begin, end, match,textureFindThingRegex)) {
        std::string type = match[1].str();
        std::string name = match[2].str();
        defaultShader->textureLocs[index] = glGetUniformLocation(defaultShader->program, name.c_str());
        index++;
        begin = match.suffix().first;
    }

    shader = defaultShader;

    dTInd = 0;
    depthTexture.resize(1);
    glGenTextures(1, &depthTexture[0]);

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
    glGenTextures(1, &depthRTexture);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
}


void Bento::initSound(){
    aldevice = alcOpenDevice(nullptr);
    context = alcCreateContext(aldevice, nullptr);
    if (!context)alcCloseDevice(aldevice);
    alcMakeContextCurrent(context);
    ALenum error = alGetError();
}


void Bento::setClearColor(glm::vec4 col){
    clearColor = col;
}

void Bento::focus(){
    glfwFocusWindow(window);
}

void Bento::setVerticesDirect(const std::vector<glm::vec3>& vs) {
    vertices = vs;    
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
}

void Bento::setNormalsDirect(const std::vector<glm::vec3>& ns) {
    normals = ns;
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
}

void Bento::setUvsDirect(const std::vector<glm::vec2>& uv) {
    uvs = uv;
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);
}

void Bento::setVertices(class vertexBuffer vs) {
    vertices = vs.getBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
}

void Bento::setNormals(class normalBuffer ns) {
    normals = ns.getBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
}

void Bento::setUvs(class uvBuffer us) {
    uvs = us.getBuffer();
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);
}

void Bento::bindTexture(Texture *texture, int ind) {
    glUniform1i(shader->textureLocs[ind], ind);
    glActiveTexture(GL_TEXTURE0+ind);
    glBindTexture(GL_TEXTURE_2D, texture->getTexture());
}

void Bento::unbindTexture() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Bento::predraw() {
    for(int i = GLFW_KEY_SPACE; i <= GLFW_KEY_LAST; ++i)prevKeyStates[i] = getKey(i);
    for(int i = GLFW_MOUSE_BUTTON_1; i <= GLFW_MOUSE_BUTTON_LAST; ++i)prevMouseStates[i] = getMouse(i);
    glfwPollEvents();
    glClearColor(clearColor.x,clearColor.y,clearColor.z,clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i) {
        if (glfwJoystickPresent(i)) {
            buttons[i] = glfwGetJoystickButtons(i, &buttonCount);
            axes[i] = glfwGetJoystickAxes(i, &axisCount);
        }
    }
}
void Bento::setShader(Shader* shd) {
    glUseProgram(shd->program);
    useDefShader = shd==defaultShader;
    shader = shd;
}

void Bento::draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);
}

void Bento::render() {
    glfwSwapBuffers(window);
}

void Bento::setActiveTextures(int start, int end){
    startRT = start;
    endRT = end+1;
    for(int i = texture.size(); i < end; i++){
        normalizedTextures.push_back(true);
    }
}
void Bento::setActiveTextures(int ind){
    startRT = ind;
    endRT = ind+1;
    for(int i = texture.size(); i < ind; i++){
        normalizedTextures.push_back(true);
    }
}

void Bento::setActiveDepthTexture(int ind){
    if(depthTexture.size()<ind){
        depthTexture.resize(ind+1);
    }
    dTInd = ind;
}

void Bento::setActiveAttachments(int start, int end){
    startAtt = start;
    endAtt = end+1;
}
void Bento::setActiveAttachments(int ind){
    startAtt = ind;
    endAtt = ind+2;
}

void Bento::predrawTex(int width, int height) {
    glCullFace(GL_FRONT);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    if(!depthTexture[dTInd]){
        glGenTextures(1, &depthTexture[dTInd]);
    }
    glBindTexture(GL_TEXTURE_2D, depthTexture[dTInd]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthTexture[dTInd], 0);
    for(int i = 0; i < (endRT-startRT); i++){
        if (startRT+i >= texture.size()) {
            texture.resize(startRT+i + 1);
            glGenTextures(1, &texture[startRT+i]);
        }
        glBindTexture(GL_TEXTURE_2D, texture[startRT+i]);
        if(normalizedTextures[i]){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        }else{
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+startAtt+1+i, GL_TEXTURE_2D, texture[startRT+i], 0);
    }

    glBindTexture(GL_TEXTURE_2D, depthRTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthRTexture, 0);
    glGetIntegerv(GL_VIEWPORT, tvp);
    glViewport(0, 0, width, height);
    
    GLuint attachments[(endAtt-startAtt)+1];
    attachments[0] = GL_COLOR_ATTACHMENT0;

    for(int i = 0; i < (endAtt-startAtt)+1; i++) {
        attachments[i] = GL_COLOR_ATTACHMENT0 + startAtt + i;
    }
    glDrawBuffers(endAtt-startAtt, attachments);

    float clearColorDepth[4] = {0.0,0.0,0.0,1.0};
    glClearBufferfv(GL_COLOR,0,clearColorDepth);
    
    float clearColorValues[4] = {clearColor.x, clearColor.y, clearColor.z, clearColor.w};
    for(int i = startAtt+1; i < endAtt; i++) {
        glClearBufferfv(GL_COLOR, i, clearColorValues);
    }
    float clearDepth = 1.0f;
    glClearBufferfv(GL_DEPTH, 0, &clearDepth);
}

void Bento::drawTex() {
    GLuint attachments[(endAtt-startAtt) + 1];
    attachments[0] = GL_COLOR_ATTACHMENT0;
    for(int i = 1; i < (endAtt-startAtt) + 1; i++){
        attachments[i] = GL_COLOR_ATTACHMENT0+i+startAtt;
    }
    glDrawBuffers(endAtt-startAtt, attachments);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);
}

void Bento::renderTex(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(tvp[0], tvp[1], tvp[2], tvp[3]);
    glCullFace(GL_BACK);
}

void Bento::renderToTex(Texture*& tex,int ind){
    tex = new Texture(texture[ind]);
}

void Bento::renderDepthToTex(Texture*& tex,int ind){
    tex = new Texture(depthTexture[ind]);
}

void Bento::renderToTex(Texture*& tex1, Texture*& tex2,int ind){
    tex1 = new Texture(texture[ind]);
    tex2 = new Texture(texture[ind+1]);
}

void Bento::renderToTex(Texture*& tex1, Texture*& tex2, Texture*& tex3,int ind){
    tex1 = new Texture(texture[ind]);
    tex2 = new Texture(texture[ind+1]);
    tex3 = new Texture(texture[ind+2]);
}
void Bento::normalizeTexture(int index,bool normalized){
    normalizedTextures[index] = normalized;
}
bool Bento::isRunning(){return !glfwWindowShouldClose(window);}

// #### INPUT ####
void Bento::exit() {
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
    std::exit(0);
}

void Bento::toggleFullscreen() {
    
}
bool Bento::isWindowFocused() {
    return glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0;
}
// #### INPUT ####
bool Bento::getKey(int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}
bool Bento::getMouse(int mouse) {
    return glfwGetMouseButton(window, mouse) == GLFW_PRESS;
}
bool Bento::getKeyDown(int key) {
    bool curr = getKey(key);
    bool prev = prevKeyStates[key];
    return curr && !prev;
}
bool Bento::getKeyUp(int key) {
    bool curr = getKey(key);
    bool prev = prevKeyStates[key];
    return !curr && prev;
}
bool Bento::getMouseDown(int button) {
    bool curr = getMouse(button);
    bool prev = prevMouseStates[button];
    return curr && !prev;
}
bool Bento::getMouseUp(int button) {
    bool curr = getMouse(button);
    bool prev = prevMouseStates[button];
    return !curr && prev;
}


double Bento::getScroll(int wheel){
    if(wheel == 0){
        float temp = wheelY;
        wheelY = 0;
        return temp;
    }
    if(wheel == 1){
        float temp = wheelX;
        wheelX = 0;
        return temp;
    }
    return 0;
}

// #### MOUSE AND WINDOWS ####
glm::vec2 Bento::getWindowSize() {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    return glm::vec2(width, height);
}

glm::vec2 Bento::getWindowPos() {
    int x, y;
    glfwGetWindowPos(window, &x, &y);
    return glm::vec2(x, y);
}

void Bento::setMouseCursor(bool hide, int cursor) {
    if (hide) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

glm::vec2 Bento::getMousePosition() {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return glm::vec2((int)x,(int)y)+getWindowPos();
}

void Bento::setMousePosition(glm::vec2 pos, bool needsFocus) {
    glm::vec2 windowPos = getWindowPos();
    glfwSetCursorPos(window, pos.x-windowPos.x, pos.y-windowPos.y);
    if (needsFocus) {
        glfwFocusWindow(window);
    }
}

glm::vec2 Bento::getControllerAxis(int controller, JoystickType joystick) {
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
bool Bento::getControllerButton(int controller, ButtonType button){
    return buttons[controller][button] == GLFW_PRESS;
}

void Bento::setWindowPos(glm::vec2 pos) {
    glfwSetWindowPos(window, pos.x, pos.y);
}

glm::vec2 Bento::getDisplaySize() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return glm::vec2(width, height);
}



void Bento::initImgui() {
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
        wheelX = xoffset;
        wheelY = yoffset;
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    });

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
    });
}


void Bento::imguiNewFrame() {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = ImVec2((float)width, (float)height);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

void Bento::setAmbientColor(glm::vec3 ambient){amb = ambient;}



void Bento::imguiRender() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


std::string Bento::getFramework(){
    return "OpenGL";
}


Shader* Bento::getDefaultShader(){
    return defaultShader;
}

std::unordered_map<std::string, int> extractUniforms(GLuint program) {
    std::unordered_map<std::string, int> uniformMap;
    
    GLint numUniforms;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);
    
    for (GLint i = 0; i < numUniforms; i++) {
        char name[128];
        GLint size;
        GLenum type;
        glGetActiveUniform(program, i, sizeof(name), nullptr, &size, &type, name);
        int location = glGetUniformLocation(program, name);
        if (location != -1) {
            uniformMap[name] = location;
        }
    }
    
    return uniformMap;
}

std::string Bento::getUni(){
    std::string out;
    auto uniforms = extractUniforms(shader->program);
    for (const auto& [name, index] : uniforms) {
        out.append(name);
        out.append(" : ");
        out.append(std::to_string(index));
        out.append("\n");
    }
    return out;
}

std::string Shader::getUni(){
    std::string out;
    auto uniforms = extractUniforms(program);
    for (const auto& [name, index] : uniforms) {
        out.append(name);
        out.append(" : ");
        out.append(std::to_string(index));
        out.append("\n");
    }
    return out;
}

Shader::Shader(std::string vertPath, std::string fragPath){
    std::cout << "compiling "+vertPath+" and "+fragPath;
    size_t lastSlash = vertPath.find_last_of("/\\");
    std::string vertDir = "";
    std::string vertFilename = "";
    if (lastSlash != std::string::npos) {
        vertDir = vertPath.substr(0, lastSlash);
        vertFilename = vertPath.substr(lastSlash, vertPath.rfind('.') - lastSlash);
    }
    lastSlash = fragPath.find_last_of("/\\");
    std::string fragDir = "";
    std::string fragFilename = "";
    if (lastSlash != std::string::npos) {
        fragDir = fragPath.substr(0, lastSlash);
        fragFilename = fragPath.substr(lastSlash, fragPath.rfind('.') - lastSlash);
    }

    #ifdef CONVERT
        system(("glslangValidator -V --quiet " + vertPath + " -o "+vertDir+vertFilename+".vert.spv").c_str());
        system(("glslangValidator -V --quiet " + fragPath + " -o "+fragDir+fragFilename+".frag.spv").c_str());
        system(("spirv-cross "+vertDir+vertFilename+".vert.spv --version 330 --output " +vertDir+"/cache"+vertFilename + ".vs").c_str());
        system(("spirv-cross "+fragDir+fragFilename+".frag.spv --version 330 --output " +fragDir+"/cache"+fragFilename + ".fs").c_str());
        system(("./bento/shaders/330c "+vertDir+"/cache"+vertFilename+".vs "+fragDir+"/cache"+fragFilename+".fs").c_str());
        system(("rm "+vertDir+vertFilename+".vert.spv "+fragDir+fragFilename+".frag.spv").c_str());
        std::cout << " to "+vertDir+"/cache"+vertFilename+".vs and "+fragDir+"/cache"+fragFilename+".fs\n";
    #endif
    
    vertSource = loadShaderSource(vertDir+"/cache"+vertFilename+".vs");
    fragSource = loadShaderSource(fragDir+"/cache"+fragFilename+".fs");

    size_t pos = vertSource.rfind("gl_Position");
    if (pos != std::string::npos) {
        size_t endPos = vertSource.find(";", pos);
        if (endPos != std::string::npos) {
            vertSource.insert(endPos + 1, "\ngl_Position.y = -gl_Position.y;");
        }
    }
    program = createShaderProgram(vertSource,fragSource);

    std::smatch match;
    auto begin = fragSource.cbegin();
    auto end = fragSource.cend();
    std::regex textureFindThingRegex(R"((sampler\w+)\s+(\w+);)");
    int index = 0;
    while (std::regex_search(begin, end, match,textureFindThingRegex)) {
        std::string type = match[1].str();
        std::string name = match[2].str();
        textureLocs[index] = glGetUniformLocation(program, name.c_str());
        index++;
        begin = match.suffix().first;
    }
}


void Bento::enable(Feature f, bool enabled){
    switch(f){
        case 0:glfwSwapInterval(0);break;
        case 1:if(enabled)glEnable(GL_DEPTH_TEST);else glDisable(GL_DEPTH_TEST);break;
        case 2:case 3://idk just do some push ups or something
        break;
    }
}