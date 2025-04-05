#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <time.h>
#include <unordered_map>
#include <vector>
#include "bento/lib/glm/glm.hpp"
#include "bento/lib/glm/gtc/matrix_transform.hpp"
#include "bento/lib/glm/gtc/type_ptr.hpp"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <random>

#include <chrono>

#include "bento/bento.h"

glm::vec3 position(0.0f,1.0f,3.0f), speed(0,0,0), maxSpeed(0,0,0);
glm::vec3 direction(0.0f,0.0f,-1.0f), right(1,0,0), up(0,1,0);

std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
float deltaTime = 0.0f, elapsedTime = 0.0f;

glm::vec2 imguiVec2ToGlm(ImVec2 v){return glm::vec2(v.x,v.y);}
ImVec2 glmVec2ToImgui(glm::vec2 v){return ImVec2(v.x,v.y);}
int min(int a,int b){return a>b?b:a;}
int max(int a,int b){return a>b?a:b;}

struct Light
{
    Light(std::string n,int i,glm::vec3 p,glm::vec3 a=glm::vec3(1.0f), glm::vec3 d=glm::vec3(1.0f), glm::vec3 s=glm::vec3(1.0f), float c =1.f,float l=0.09f,float q=0.032f):name(n),position(p),ambient(a),diffuse(d),specular(s),constant(c),linear(l),quadratic(q),index(i){}
    void add(Bento* bento){
        bento->addLight(position,ambient,diffuse,specular,constant,linear,quadratic);
    }
    void updatePosition(Bento* bento){
        bento->setLightPos(index,position);
    }
    void updateAmbient(Bento* bento){
        bento->setLightAmbients(index,ambient);
    }
    void updateDiffuse(Bento* bento){
        bento->setLightDiffuses(index,diffuse);
    }
    void updateSpecular(Bento* bento){
        bento->setLightSpeculars(index,specular);
    }
    void updateConstant(Bento* bento){
        bento->setLightConstants(index,constant);
    }
    void updateLinear(Bento* bento){
        bento->setLightLinears(index,linear);
    }
    void updateQuadratic(Bento* bento){
        bento->setLightQuads(index,quadratic);
    }
    void updateAll(Bento* bento){
        bento->setLightPos(index,position);
        bento->setLightAmbients(index,ambient);
        bento->setLightDiffuses(index,diffuse);
        bento->setLightSpeculars(index,specular);
        bento->setLightConstants(index,constant);
        bento->setLightLinears(index,linear);
        bento->setLightQuads(index,quadratic);
    }
    std::string name;
    int index;
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

int main() {
    Bento* bento = new Bento();
    bento->init("ベント",1000,1000);
    bento->focus();
    Shader* defaultShader = bento->getDefaultShader();
    
    Shader* screenShader = new Shader("./resources/shaders/screen.vert","./resources/shaders/screen.frag");


    bento->setClearColor(glm::vec4(0,0,0,1));

    bento->initImgui();

    ImFont* ogfont = ImGui::GetFont();
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF("./resources/fonts/notoSansMono/NotoSansMono-Regular.ttf",15);
    ImFont* boldFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("./resources/fonts/notoSansMono/NotoSansMono-Bold.ttf",15);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    Texture* scrnTex = new Texture("./resources/grass.png");
    Texture* colTex;
    Texture* nrmlTex;
    Texture* uvTex;
    Texture* depthTex;
    Texture* posTex;

    Texture* tex = new Texture("./resources/grass.png");
    

    std::vector<Light> lights;

    lights.emplace_back("light1",lights.size(),glm::vec3(10,1,10),glm::vec3(1,1,1),glm::vec3(1,1,1),glm::vec3(1,1,1),1.0,0.9,0.2);
    lights.emplace_back("light2",lights.size(),glm::vec3(10,3,1),glm::vec3(10,10,10),glm::vec3(1,1,1),glm::vec3(1,1,1),1.0,0.9,0.2);
    lights.emplace_back("light3",lights.size(),glm::vec3(1,1,10),glm::vec3(1,1,1),glm::vec3(1,0,0),glm::vec3(1,0.6,0.6),0.1,0.8,0.01);


    for(Light light: lights){
        light.add(bento);
    }

    std::vector<Object*> objects;


    for(int i = -25; i < 25; i+=2)
        for(int j = 0; j < 25; j+=2)
            objects.emplace_back(new Object("sword",glm::vec3(i,j,-10),"resources/sword.obj"));

    Texture *swordTex = new Texture("./resources/sword.png");//or you can do "resources/sword.png"

    Mesh *groundMesh = new Mesh("./resources/ground.obj");
    Texture *groundTex = new Texture("./resources/ground.png");

    std::vector<glm::vec3> vs, ns;
    std::vector<glm::vec2> us;
    loadOBJ("./resources/suzanne.obj",vs,us,ns);
    std::vector<float> vo;
    for(glm::vec3 v:vs)vo.push_back(v.x);


    objects.emplace_back(new Object("shphere",glm::vec3(0,10,0),"resources/hmm.obj","resources/hmm.png"));
    
    float angleX = 3.14, angleY = 0;
    float fov = 110.0f, tPDist = 10.0f, tPDistSensitivity = 0.75f;
    int pov = 0;

    glm::vec2 mousePos(0,0), lastMousePos(0,0);

    glm::vec2 windowSize(bento->getWindowSize());
    glm::vec2 viewSize(windowSize);
    glm::vec2 viewPos(0,0);
    
    while (bento->isRunning()){
        windowSize = bento->getWindowSize();

        direction = glm::vec3(cos(angleY) * sin(angleX),sin(angleY),cos(angleY) * cos(angleX));
        right = glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f));
        up = glm::cross(right,direction);
        float tilt = 0.003;
        if(pov==0){
            view = glm::lookAt(position-direction*tPDist,position-direction*(tPDist-1),up+glm::vec3(speed.x*tilt,0,speed.z*tilt));
            bento->setViewMatrix(view,position-direction*tPDist);
        }else{
            view = glm::lookAt(position,position+direction,up+glm::vec3(speed.x*tilt,0,speed.z*tilt));
            bento->setViewMatrix(view,position);
        }

        projection = glm::perspective(glm::radians(fov), fmax(viewSize.x,1.0f) / fmax(viewSize.y,1.0f), 0.01f, 1000.0f);
        bento->setProjectionMatrix(projection);

        bento->setActiveTextures(0,3);
        bento->setActiveDepthTexture(0);
        bento->setActiveAttachments(0,3);
        bento->predrawTex(viewSize.x,viewSize.y);

        bento->setShader(defaultShader);
        static float tspecular = 10.0f;
        bento->setUniform("tspecular",tspecular);


        bento->bindTexture(swordTex,0);
        for(Object* obj : objects)obj->drawTex(bento);

        bento->setVertices(groundMesh->getVertexBuffer());
        bento->setNormals(groundMesh->getNormalBuffer());
        bento->setUvs(groundMesh->getUVBuffer());
        bento->bindTexture(groundTex,0);
        model = glm::translate(glm::mat4(1.0),glm::vec3(0.0,0.1,0.0));
        bento->setModelMatrix(model);
        bento->drawTex();

        model = glm::translate(glm::mat4(1.0),glm::vec3(0.0,1.5,0.0));
        bento->setModelMatrix(model);
        for(int i = 0; i < vs.size(); i++)vs[i].x = vo[i]*1.25+sin(elapsedTime+vs[i].y*4)*0.125;

        bento->setVerticesDirect(vs);
        bento->setNormalsDirect(ns);
        bento->setUvsDirect(us);

        bento->drawTex();

        bento->renderTex();

        bento->renderToTex(colTex,0);
        bento->renderToTex(nrmlTex,1);
        bento->renderToTex(uvTex,2);
        bento->renderToTex(posTex,3);

        bento->renderDepthToTex(depthTex,0);
        static int kernelSize = 16;
        static float radius = 0.5;
        static float bias = 0.025;
        static float intensity = 2.0;
        static float THRESHOLD = 0.1;
        static float KNEE = 0.7;


        bento->setActiveTextures(4);
        bento->setActiveDepthTexture(1);
        bento->setActiveAttachments(0);
        bento->predrawTex(viewSize.x,viewSize.y);

        bento->setShader(screenShader);
        bento->setUniform("kernelSize",kernelSize);
        bento->setUniform("radius",radius);
        bento->setUniform("bias",bias);
        bento->setUniform("intensity",intensity);
        bento->setUniform("projection",projection);
        bento->setUniform("THRESHOLD",THRESHOLD);
        bento->setUniform("KNEE",KNEE);
        
        bento->bindTexture(colTex,0);
        bento->bindTexture(nrmlTex,1);
        bento->bindTexture(uvTex,2);
        bento->bindTexture(depthTex,3);
        bento->bindTexture(posTex,4);


        std::vector<glm::vec3> screenVertices;
        screenVertices.push_back(glm::vec3(1,1,0));
        screenVertices.push_back(glm::vec3(-1,-1,0));
        screenVertices.push_back(glm::vec3(1,-1,0));
        screenVertices.push_back(glm::vec3(1,1,0));
        screenVertices.push_back(glm::vec3(-1,1,0));
        screenVertices.push_back(glm::vec3(-1,-1,0));
 
        std::vector<glm::vec2> screenUVs;
        screenUVs.push_back(glm::vec2(1,1));
        screenUVs.push_back(glm::vec2(0,0));
        screenUVs.push_back(glm::vec2(1,0));
        screenUVs.push_back(glm::vec2(1,1));
        screenUVs.push_back(glm::vec2(0,1));
        screenUVs.push_back(glm::vec2(0,0));
        bento->setVerticesDirect(screenVertices);
        bento->setNormalsDirect(screenVertices);
        bento->setUvsDirect(screenUVs);

        bento->drawTex();

        bento->renderTex();

        bento->renderToTex(scrnTex,4);
        bento->renderDepthToTex(depthTex,0);

        bento->predraw();
        bento->setShader(defaultShader);

        bento->imguiNewFrame();

        ImGui::PushFont(font);

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(ImVec2(windowSize.x,windowSize.y));
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        
        ImGuiStyle oldStyle = ImGui::GetStyle();
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        ImGui::Begin("dockspace", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::DockSpace(ImGui::GetID("dockspace"), ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        ImGui::End();

        ImGui::GetStyle() = oldStyle;
        ImGui::PopStyleVar(2);


        static bool middleMouseToLook = true;
        static bool rightMouseToLook = false;
        static bool clickLookToFocus = true;

        ImGui::Begin("viewport");
        ImVec2 tsize = ImGui::GetWindowSize();
        ImVec2 tpos = ImGui::GetWindowPos();
        viewPos = glm::vec2(tpos.x,tpos.y);
        viewSize = glm::vec2(tsize.x,tsize.y-ImGui::GetFrameHeight());
        ImGui::Image((ImTextureID)scrnTex->getTexture(),ImVec2(viewSize.x,viewSize.y),ImVec2(0,1),ImVec2(1,0));
        tpos.y += ImGui::GetFrameHeight();

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        float centerX = tsize.x / 2.0f;
        float centerY = tsize.y / 2.0f;
        float crosshairSize = 10.0f;
        float lineWidth = 1.5f;
        float gapSize = 4.0f;
        float dotSize = 2.0f;
        drawList->AddText(ImVec2(5+viewPos.x,15+viewPos.y),ImColor(255,255,255),(std::to_string(1/deltaTime)).c_str());
        drawList->AddText(ImVec2(5+viewPos.x,30+viewPos.y),ImColor(255,255,255),screenShader->getUni().c_str());
        centerX = tsize.x-min(0.075*tsize.x,0.075*tsize.y)+viewPos.x-2;
        centerY = tsize.y-min(0.075*tsize.x,0.075*tsize.y)+viewPos.y-2;
        float compassSize = min(0.075*tsize.x,0.075*tsize.y);
        drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX,centerY+compassSize*sin(angleY-pi/2)),ImColor(255,0,0),2);
        drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX+compassSize*cos(-angleX),centerY+compassSize*sin(-angleX)*cos(angleY-pi/2)),ImColor(0,255,0),2);
        drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX+compassSize*cos(-angleX+pi/2),centerY+compassSize*sin(-angleX+pi/2)*cos(angleY-pi/2)),ImColor(0,0,255),2);
        drawList->AddEllipse({centerX,centerY},ImVec2(10,10),compassSize*cos(angleY-pi/2),ImColor(150,150,150));
        drawList->AddCircle({centerX,centerY},compassSize,ImColor(200,200,200));
        bool viewportHover = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        bool viewportFocus = ImGui::IsWindowFocused();

        if(clickLookToFocus && viewportHover && ((middleMouseToLook&&bento->getMouse(MOUSE_MIDDLE)) || (rightMouseToLook&&bento->getMouse(MOUSE_RIGHT)))) {
            ImGui::SetWindowFocus();
        }

        ImGui::End();


        ImGui::Begin("viewport ext");
        tsize = ImGui::GetWindowSize();
        ImVec2 imageSize = ImVec2((tsize.x - ImGui::GetFrameHeight()) / 2, (tsize.y - ImGui::GetFrameHeight()) / 2);
        ImGui::Image((ImTextureID)colTex->getTexture(), imageSize);
        ImGui::SameLine();
        ImGui::SetCursorPosX(imageSize.x);
        ImGui::Image((ImTextureID)nrmlTex->getTexture(), imageSize);
        ImGui::SetCursorPosY(imageSize.y+15);
        ImGui::SetCursorPosX(0);
        ImGui::Image((ImTextureID)uvTex->getTexture(), imageSize);
        ImGui::SameLine();
        ImGui::SetCursorPosX(imageSize.x);
        ImGui::Image((ImTextureID)depthTex->getTexture(), imageSize);
        
        ImGui::End();

        //ImDrawList* drawList = ImGui::GetBackgroundDrawList();


        static char commandBuffer[256] = "";
        static std::string outputBuffer;
        static bool scrollToBottom = false;

        ImGui::Begin("terminal");
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f,0.f,0.f, 1.f));
        tsize = ImGui::GetContentRegionAvail();
        ImGui::BeginChild("##output", ImVec2(tsize.x, tsize.y - 29), true);
        
        ImGui::TextUnformatted(outputBuffer.c_str());

        if (scrollToBottom) {
            ImGui::SetScrollHereY(1.0f);
            scrollToBottom = false;
        }
        
        ImGui::EndChild();
        ImGui::PopStyleColor();
        
        if (ImGui::InputText("##cmd", commandBuffer, sizeof(commandBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {

            scrollToBottom = true;
            ImGui::SetKeyboardFocusHere(-1);

            outputBuffer += "> " + std::string(commandBuffer) + "\n";

            FILE* pipe = popen(commandBuffer, "r");
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                outputBuffer += buffer;
            }
            pclose(pipe);
            memset(commandBuffer, 0, sizeof(commandBuffer));
        }

        ImGui::End();

        ImGui::PopStyleVar(2);

        ImGui::Begin("settings",nullptr,0);

        static bool freeCam = true;
        static bool ijklAsLookKeys = false;
        static bool scrollToChangeDist = true;
        static bool shiftToMove = true;
        static bool wasdToMove = false;
        static bool tPDistInvert = false;

        static int approachSelection = 0;
        static float approachDamp = 0.01f;
        static float approachBase = 2.0f;

        static bool shiftToGoFast = true;
        static float fastSpeed = 0.3f;
        static float slowSpeed = 0.1f;

        static bool shiftLockMouse = false;
        static bool shiftWrapMouse = true;

        static bool lockMouse = false;
        static bool wrapMouse = true;

        static int wrapSelection = 0;
        static int shiftWrapSelection = 0;

        static bool invShiftMoveX = true;
        static float shiftMoveSensX = 0.002f;
        static bool invShiftMoveY = false;
        static float shiftMoveSensY = 0.002f;
        static float shiftMoveProp = 0.75f;

        ImGui::DragInt("kernel size", &kernelSize);
        ImGui::DragFloat("radius", &radius, 0.1f);
        ImGui::DragFloat("bias", &bias, 0.1f);
        ImGui::DragFloat("intensity", &intensity, 0.1f);

        ImGui::DragFloat("THRESHOLD", &THRESHOLD);
        ImGui::DragFloat("KNEE", &KNEE, 0.1f);
        
        if (ImGui::CollapsingHeader("movement settings")) {

            ImGui::DragFloat("fov", &fov, 0.1f);

            ImGui::Checkbox("free cam", &freeCam);
            ImGui::Checkbox("ijkl as turn keys", &ijklAsLookKeys);
            if(ImGui::Checkbox("middle mouse to turn", &middleMouseToLook))rightMouseToLook = false;
            if(ImGui::Checkbox("right mouse to turn", &rightMouseToLook))middleMouseToLook = false;
            ImGui::Checkbox("click look button to focus", &clickLookToFocus);
            if(ImGui::Checkbox("lock mouse",&lockMouse)){wrapMouse=false;}
            if(ImGui::Checkbox("wrap mouse",&wrapMouse)){lockMouse=false;}
            const char* wrapOptions[3] = {"viewport","window","display"};
            if(!wrapMouse)ImGui::BeginDisabled();
            ImGui::Combo("wrap bounds",&wrapSelection,wrapOptions,3);
            if(!wrapMouse)ImGui::EndDisabled();
            
            
            const char* povs[2]={"third person","first person"};
            if(ImGui::Combo("point of view",&pov,povs,2)){
                shiftToMove=(pov==0);
                wasdToMove=(pov==1);
            }
            
            ImGui::Indent(20.0f);
                
                ImGui::Checkbox("wasd to move",&wasdToMove);

                if(!wasdToMove)ImGui::BeginDisabled();
                ImGui::Checkbox("fast shift",&shiftToGoFast);
                if(!shiftToGoFast)ImGui::BeginDisabled();
                ImGui::DragFloat("fast speed",&fastSpeed,0.001f,0.001f,FLT_MAX);
                ImGui::DragFloat("slow speed",&slowSpeed,0.001f,0.001f,FLT_MAX);
                if(!wasdToMove)ImGui::EndDisabled();
                if(!shiftToGoFast)ImGui::EndDisabled();

                ImGui::Checkbox("shift to move", &shiftToMove);

                if(!shiftToMove)ImGui::BeginDisabled();
                    ImGui::Checkbox("invert X",&invShiftMoveX);
                    ImGui::DragFloat("X sensitivity",&shiftMoveSensX,0.001f,0.001f,FLT_MAX);
                    ImGui::Checkbox("invert Y",&invShiftMoveY);
                    ImGui::DragFloat("Y sensitivity",&shiftMoveSensY,0.001f,0.001f,FLT_MAX);
                    ImGui::DragFloat("dist proportionality",&shiftMoveProp,0.005f,0.0f,FLT_MAX);
                    if(ImGui::Checkbox("lock mouse##2",&shiftLockMouse)){shiftWrapMouse=false;}
                    if(ImGui::Checkbox("wrap mouse##2",&shiftWrapMouse)){shiftLockMouse=false;}
                    if(!shiftWrapMouse)ImGui::BeginDisabled();
                    ImGui::Combo("wrap bounds##2",&shiftWrapSelection,wrapOptions,3);
                    if(!shiftWrapMouse)ImGui::EndDisabled();
                if(!shiftToMove)ImGui::EndDisabled();

                if(pov != 0)ImGui::BeginDisabled();

                    ImGui::PopFont();
                    ImGui::PushFont(boldFont);
                    ImGui::Text("third person settings");
                    ImGui::PopFont();
                    ImGui::PushFont(font);

                    ImGui::Checkbox("scroll changes distance", &scrollToChangeDist);
                    if(!scrollToChangeDist)ImGui::BeginDisabled();
                    ImGui::Checkbox("invert scroll", &tPDistInvert);
                    const char* approachOptions[3] = {"log","clamp","off"};
                    ImGui::Combo("scroll approach",&approachSelection,approachOptions,3);
                    if(approachSelection == 0){
                        ImGui::DragFloat("damping",&approachDamp,0.0001f,0.01f,FLT_MAX);
                        ImGui::DragFloat("base",&approachBase,0.025f,1.001f,FLT_MAX);
                        ImGui::PlotLines("graph",[](void*data,int idx){return std::logf(1.0f+std::abs(idx+1)*approachDamp)/std::logf(approachBase);},NULL,300);
                    }
                    ImGui::DragFloat("scroll sensitivity",&tPDistSensitivity, 0.1f);
                    if(!scrollToChangeDist)ImGui::EndDisabled();
                if(pov != 0)ImGui::EndDisabled();

            ImGui::Unindent(20.0f);
        }
        if (ImGui::CollapsingHeader("world settings")) {
            static float clearColor[4] = {0.0f,0.0f,0.0f,1.0f};
            static float ambientColor[3] = {0.0f,0.0f,0.0f};
            ImGui::ColorPicker4("sky color", (float*)&clearColor);
            bento->setClearColor(glm::vec4(clearColor[0],clearColor[1],clearColor[2],clearColor[3]));
            static bool amCisSkC = true;
            ImGui::Checkbox("ambient color = sky color?", &amCisSkC);
            if(amCisSkC){
                bento->setAmbientColor(glm::vec3(clearColor[0],clearColor[1],clearColor[2]));
                ambientColor[0] = clearColor[0];
                ambientColor[1] = clearColor[1];
                ambientColor[2] = clearColor[2];
            }else{
                ImGui::ColorPicker3("ambient color", (float*)&ambientColor);
                bento->setAmbientColor(glm::vec3(ambientColor[0],ambientColor[1],ambientColor[2]));
            }
        }
        static bool showDemo = false;
        
        ImGui::Checkbox("show demo", &showDemo);
        
        if (ImGui::CollapsingHeader("shader settings")) {
            ImGui::DragFloat("specular amount", &tspecular, 0.1f);
        }

        ImGui::End();

        static int selectedIndex = -1;
        
        ImGui::Begin("Explorer");
        
        for (int i = 0; i < lights.size(); i++) {
            std::string label = lights[i].name + "##" + std::to_string(i);
            ImGui::ColorButton((lights[i].name+"##color" + std::to_string(i)).c_str(), ImVec4(
                lights[i].diffuse.r,
                lights[i].diffuse.g,
                lights[i].diffuse.b,
                1.0f
            ), ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
            ImGui::SameLine();
            
            if (ImGui::Selectable(label.c_str(), selectedIndex == i)) {
                selectedIndex = i;
            }
        }
        
        ImGui::End();

        ImGui::Begin("Editor");
        
        if (selectedIndex >= 0 && selectedIndex < lights.size()) {
            Light& light = lights[selectedIndex];
            
            char nameBuf[128];
            strcpy(nameBuf, light.name.c_str());
            if (ImGui::InputText("name", nameBuf, sizeof(nameBuf))) {
                light.name = nameBuf;
            }
            ImGui::BeginDisabled();
            ImGui::InputInt("index",&light.index);
            ImGui::EndDisabled();
            if(ImGui::DragFloat3("position", glm::value_ptr(light.position)))light.updatePosition(bento);
            if(ImGui::ColorEdit3("ambient", glm::value_ptr(light.ambient)))light.updateAmbient(bento);
            if(ImGui::ColorEdit3("diffuse", glm::value_ptr(light.diffuse)))light.updateDiffuse(bento);
            if(ImGui::ColorEdit3("specular", glm::value_ptr(light.specular)))light.updateSpecular(bento);
            if(ImGui::DragFloat("constant",&light.constant,0.01f))light.updateConstant(bento);
            if(ImGui::DragFloat("linear",&light.linear,0.01f))light.updateLinear(bento);
            if(ImGui::DragFloat("quadratic",&light.quadratic,0.001f))light.updateQuadratic(bento);
        }
        if(viewportFocus){
            static float moveSpeed;
            if(shiftToGoFast&&bento->getKey(KEY_LEFT_SHIFT))moveSpeed = fastSpeed;else moveSpeed = slowSpeed;

            if(wasdToMove){
                if(freeCam){
                    if(bento->getKey(KEY_W))position += direction*glm::vec3(moveSpeed);
                    if(bento->getKey(KEY_S))position -= direction*glm::vec3(moveSpeed);
                    if(bento->getKey(KEY_D))position += right*glm::vec3(moveSpeed);
                    if(bento->getKey(KEY_A))position -= right*glm::vec3(moveSpeed);
                    if(bento->getKey(KEY_E))position += up*glm::vec3(moveSpeed);
                    if(bento->getKey(KEY_Q))position -= up*glm::vec3(moveSpeed);
                }else{
                    if(bento->getKey(KEY_A))position -= glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f))*glm::vec3(moveSpeed);
                    if(bento->getKey(KEY_S))position -= glm::vec3(sin(angleX),0,cos(angleX))*glm::vec3(moveSpeed);
                    if(bento->getKey(KEY_D))position += glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f))*glm::vec3(moveSpeed);
                    if(bento->getKey(KEY_W))position += glm::vec3(sin(angleX),0,cos(angleX))*glm::vec3(moveSpeed);
                    if(bento->getKey(KEY_E))position.y += moveSpeed;
                    if(bento->getKey(KEY_Q))position.y -= moveSpeed;
                }
            }

            lights[0].position = position;
            lights[0].updatePosition(bento);

            if(ijklAsLookKeys){
                if(bento->getKey(KEY_L))angleX -= 0.1;
                if(bento->getKey(KEY_J))angleX += 0.1;
                if(bento->getKey(KEY_I))angleY += 0.1;
                if(bento->getKey(KEY_K))angleY -= 0.1;
            }

            static bool lookClicked = false;
            static bool waitForLookRelease = false;

            int lookKey = MOUSE_MIDDLE;
            if(rightMouseToLook)lookKey = MOUSE_RIGHT;

            mousePos = bento->getMousePosition();

            if(bento->getMouse(lookKey)){
                if(!lookClicked){
                    if(!viewportHover)waitForLookRelease = true;
                    lastMousePos = bento->getMousePosition();
                    lookClicked = true;
                }
                if(!waitForLookRelease){
                    if(bento->getKey(KEY_LEFT_SHIFT)&&shiftToMove){
                        position += right*(lastMousePos.x-mousePos.x)*glm::vec3((invShiftMoveX?1:-1)*shiftMoveSensX)*(tPDist*shiftMoveProp+1);
                        position += up*(lastMousePos.y-mousePos.y)*glm::vec3((invShiftMoveY?1:-1)*shiftMoveSensY)*(tPDist*shiftMoveProp+1);
                        if(shiftLockMouse)bento->setMousePosition(lastMousePos);
                        else lastMousePos = mousePos;
                        if(shiftWrapMouse){
                            //const char* wrapOptions[3] = {"viewport","window","display"};
                            glm::vec2 o,m;
                            switch(shiftWrapSelection){
                                case 0:o=bento->getWindowPos()+viewPos;m=viewSize;break;
                                case 1:o=bento->getWindowPos();m=bento->getWindowSize();break;
                                case 2:o=glm::vec2(0,0);m=bento->getDisplaySize();break;
                            }
                            if(mousePos.x<=o.x){lastMousePos=glm::vec2(o.x+m.x-2,mousePos.y);bento->setMousePosition(glm::vec2(o.x+m.x-2,mousePos.y));}
                            if(mousePos.y<=o.y){lastMousePos=glm::vec2(mousePos.x,o.y+m.y-2);bento->setMousePosition(glm::vec2(mousePos.x,o.y+m.y-2));}
                            if(mousePos.x>=o.x+m.x-1){lastMousePos=glm::vec2(o.x+1,mousePos.y);bento->setMousePosition(glm::vec2(o.x+1,mousePos.y));}
                            if(mousePos.y>=o.y+m.y-1){lastMousePos=glm::vec2(mousePos.x,o.y+1);bento->setMousePosition(glm::vec2(mousePos.x,o.y+1));}
                        }
                    }else{
                        angleX += (lastMousePos.x-mousePos.x)*0.005;
                        angleY += (lastMousePos.y-mousePos.y)*0.005;
                        if(lockMouse)bento->setMousePosition(lastMousePos);
                        else lastMousePos = mousePos;
                        if(wrapMouse){
                            glm::vec2 o,m;
                            switch(wrapSelection){
                                case 0:o=bento->getWindowPos()+viewPos;m=viewSize;break;
                                case 1:o=bento->getWindowPos();m=bento->getWindowSize();break;
                                case 2:o=glm::vec2(0,0);m=bento->getDisplaySize();break;
                            }
                            if(mousePos.x<=o.x){lastMousePos=glm::vec2(o.x+m.x-2,mousePos.y);bento->setMousePosition(glm::vec2(o.x+m.x-2,mousePos.y));}
                            if(mousePos.y<=o.y){lastMousePos=glm::vec2(mousePos.x,o.y+m.y-2);bento->setMousePosition(glm::vec2(mousePos.x,o.y+m.y-2));}
                            if(mousePos.x>=o.x+m.x-1){lastMousePos=glm::vec2(o.x+1,mousePos.y);bento->setMousePosition(glm::vec2(o.x+1,mousePos.y));}
                            if(mousePos.y>=o.y+m.y-1){lastMousePos=glm::vec2(mousePos.x,o.y+1);bento->setMousePosition(glm::vec2(mousePos.x,o.y+1));}
                        }
                    }
                }
            }else{
                lookClicked = false;
                waitForLookRelease = false;
            }
        }
        if(viewportHover&&scrollToChangeDist){
            tPDist+=bento->getScroll(0)*tPDistSensitivity*(tPDistInvert?1:-1)*(approachSelection==0?(std::log(1.0f + std::abs(tPDist) * approachDamp)/std::log(approachBase)):1);
            if(approachSelection==1)tPDist = fmax(tPDist,0.0f);
            if(approachSelection==0)tPDist = fmax(tPDist,0.001f);
        }
        
        ImGui::End();

        if(showDemo)ImGui::ShowDemoWindow(&showDemo);

        ImGui::PopFont();

        bento->imguiRender();

        bento->render();
        
        elapsedTime+=1.0f / 70.0f;
        auto currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
    }
    bento->exit();
    delete bento;
    return 0;
}