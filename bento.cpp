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

    bento->setClearColor(glm::vec4(0,0,0,1));

    bento->initImgui();

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    Texture* scrnTex;

    Texture* tex = new Texture("./resources/grass.png");

    std::vector<Light> lights;

    lights.emplace_back("light1",lights.size(),glm::vec3(10,1,10),glm::vec3(1,1,1),glm::vec3(1,1,1),glm::vec3(1,1,1),1.0,0.9,0.2);
    lights.emplace_back("light2",lights.size(),glm::vec3(10,3,1),glm::vec3(10,10,10),glm::vec3(1,1,1),glm::vec3(1,1,1),1.0,0.9,0.2);
    lights.emplace_back("light3",lights.size(),glm::vec3(1,1,10),glm::vec3(1,1,1),glm::vec3(1,0,0),glm::vec3(1,0.6,0.6),0.1,0.8,0.01);

    for(Light light: lights){
        light.add(bento);
    }

    std::vector<Object*> objects;
    //objects.emplace_back(new Object("sword",glm::vec3(0,0,0),"resources/sword.obj"));


    for(int i = -25; i < 25; i+=2)
        for(int j = -25; j < 25; j+=2)
            objects.emplace_back(new Object("sword",glm::vec3(i,0,j),"resources/sword.obj"));

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
    float fov = 110.0f;

    glm::vec2 mousePos(0,0), lastMousePos(0,0);

    glm::vec2 windowSize(bento->getWindowSize());
    glm::vec2 viewSize(windowSize*glm::vec2(0.5));
    
    while (bento->isRunning()){
        windowSize = bento->getWindowSize();

        direction = glm::vec3(cos(angleY) * sin(angleX),sin(angleY),cos(angleY) * cos(angleX));
        right = glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f));
        up = glm::cross(right,direction);
        float tilt = 0.003;
        view = glm::lookAt(position,position+direction,up+glm::vec3(speed.x*tilt,0,speed.z*tilt));

        projection = glm::perspective(glm::radians(fov), fmax(viewSize.x,1.0f) / fmax(viewSize.y,1.0f), 0.01f, 1000.0f);
        bento->setViewMatrix(view,position);
        bento->setProjectionMatrix(projection);


        bento->predrawTex(viewSize.x,viewSize.y);

        bento->bindTexture(swordTex);
        for(Object* obj : objects)obj->drawTex(bento);

        bento->setVertices(groundMesh->getVertexBuffer());
        bento->setNormals(groundMesh->getNormalBuffer());
        bento->setUvs(groundMesh->getUVBuffer());
        bento->bindTexture(groundTex);
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

        scrnTex = bento->renderTex();

        bento->predraw();
        

        bento->imguiNewFrame();

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
        ImVec2 size = ImGui::GetWindowSize();
        viewSize = glm::vec2(size.x,size.y-13);//windowSize*glm::vec2(0.5));
        ImGui::Image((ImTextureID)scrnTex->getTexture(),ImVec2(viewSize.x,viewSize.y));

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetWindowPos();

        float centerX = size.x / 2.0f;
        float centerY = size.y / 2.0f;
        float crosshairSize = 10.0f;
        float lineWidth = 1.5f;
        float gapSize = 4.0f;
        float dotSize = 2.0f;
        drawList->AddText(ImVec2(5+p.x,15+p.y),ImColor(255,255,255),(std::to_string(1/deltaTime)).c_str());
        centerX = size.x-min(0.2*size.x,0.2*size.y)+p.x;
        centerY = size.y-min(0.2*size.x,0.2*size.y)+p.y;
        float compassSize = min(0.15*size.x,0.15*size.y);
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

        //ImDrawList* drawList = ImGui::GetBackgroundDrawList();


        static char commandBuffer[256] = "";
        static std::string outputBuffer;
        static bool scrollToBottom = false;

        ImGui::Begin("terminal");
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f,0.f,0.f, 1.f));
        size = ImGui::GetContentRegionAvail();
        ImGui::BeginChild("##output", ImVec2(size.x, size.y - 29), true);
        
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

        ImGui::Text("settings");

        static bool freeCam = true;
        static bool ijklAsLookKeys = false;

        if (ImGui::CollapsingHeader("movement settings")) {
            ImGui::DragFloat("fov", &fov, 0.1f);
            ImGui::Checkbox("free cam", &freeCam);
            ImGui::Checkbox("ijkl as turn keys", &ijklAsLookKeys);
            if(ImGui::Checkbox("middle mouse to turn", &middleMouseToLook)){
                rightMouseToLook = false;
            }
            if(ImGui::Checkbox("right mouse to turn", &rightMouseToLook)){
                middleMouseToLook = false;
            }
            ImGui::Checkbox("click look button to focus", &clickLookToFocus);
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
            if(bento->getKey(KEY_LEFT_SHIFT))moveSpeed = 0.3f;else moveSpeed = 0.1f;

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

            if(ijklAsLookKeys){
                if(bento->getKey(KEY_L))angleX -= 0.1;
                if(bento->getKey(KEY_J))angleX += 0.1;
                if(bento->getKey(KEY_I))angleY += 0.1;
                if(bento->getKey(KEY_K))angleY -= 0.1;
            }

            static bool middleClicked = false;
            static bool waitForMiddleRelease = false;
            mousePos = bento->getMousePosition();
            if(bento->getMouse(MOUSE_MIDDLE) && middleMouseToLook){
                if(!middleClicked){
                    if(!viewportHover)waitForMiddleRelease = true;
                    lastMousePos = bento->getMousePosition();
                    middleClicked = true;
                }
                if(!waitForMiddleRelease){
                    angleX += (lastMousePos.x-mousePos.x)*0.005;
                    angleY += (lastMousePos.y-mousePos.y)*0.005;
                    bento->setMousePosition(lastMousePos);
                }
            }else{
                middleClicked = false;
                waitForMiddleRelease = false;
            }

            static bool rightClicked = false;
            static bool waitForRightRelease = false;
            mousePos = bento->getMousePosition();
            if(bento->getMouse(MOUSE_RIGHT) && rightMouseToLook){
                if(!rightClicked){
                    if(!viewportHover)waitForRightRelease = true;
                    lastMousePos = bento->getMousePosition();
                    rightClicked = true;
                }
                if(!waitForRightRelease){
                    angleX += (lastMousePos.x-mousePos.x)*0.005;
                    angleY += (lastMousePos.y-mousePos.y)*0.005;
                    bento->setMousePosition(lastMousePos);
                }
            }else{
                rightClicked = false;
                waitForRightRelease = false;
            }
        }
        
        ImGui::End();

        if(showDemo)ImGui::ShowDemoWindow(&showDemo);

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