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

#include "bento/bento.h"

/*

THIS IS AN EXAMPLE

this is an example that can be built that can be used freely or as a model
                               ^
                               (run.bat bento example\ \(go\ here\)/bento.cpp) at bento/
                               (sh runlinux.sh bento example\ \(go\ here\)/bento.cpp) at bento/
                               (sh run.sh -metal bento example\ \(go\ here\)/bento.cpp) at bento/

*/

void processInputs(Bento *bento, glm::vec3 &position, glm::vec2 &lastMousePos, float &angleX, float &angleY, float speed);

glm::vec2 rightOffset, leftOffset;

int main() {
    Bento *bento = new Bento();
    bento->init("ベント",1000,1000);
    bento->focus();

    bento->initImgui();

    float elapsedTime = 0.0f;

    std::vector<Object*> objects;
    objects.emplace_back(new Object("sword",glm::vec3(0,0,0),"resources/sword.obj"));


    for(int i = -50; i < 50; i+=2)
        for(int j = -50; j < 50; j+=2)
                objects.emplace_back(new Object("sword",glm::vec3(i,0,j),"resources/sword.obj"));

    Texture *swordTex = new Texture("resources/sword.png");

    Mesh *groundMesh = new Mesh("resources/ground.obj");
    Texture *groundTex = new Texture("resources/grass.png");


    objects.emplace_back(new Object("shphere",glm::vec3(0,10,0),"resources/hmm.obj","resources/hmm.png"));
    


    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    float angleX = 3.14, angleY = 0;
    int ticks = 0;
    glm::mat4 projection = glm::mat4(1.0f);
    glm::vec3 position(0.0f,1.0f,3.0f);
    glm::vec3 direction(0.0f,0.0f,-1.0f), right(1,0,0), up(0,1,0);
    glm::vec2 displaySize = bento->getDisplaySize();
    glm::vec2 mousePos = bento->getMousePosition(), lastMousePos = bento->getMousePosition();
    leftOffset = bento->getControllerAxis(0,GAMEPAD_JOYSTICK_LEFT);
    rightOffset = bento->getControllerAxis(0,GAMEPAD_JOYSTICK_RIGHT);
    glm::vec2 windowSize;
    bento->setMousePosition(glm::vec2(500,500));

    //dear imgui

    bool showDemoWindow = true;

    while (bento->isRunning()){
        bento->predraw();

        float speed = 0.1;
        
        processInputs(bento,position,lastMousePos,angleX,angleY,speed);

        direction = glm::vec3(cos(angleY) * sin(angleX),sin(angleY),cos(angleY) * cos(angleX));
        right = glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f));
        up = glm::cross( right, direction );
        view = glm::lookAt(position,position+direction,up);
        windowSize = bento->getWindowSize();
        projection = glm::perspective(glm::radians(45.0f), windowSize.x / windowSize.y, 0.1f, 100.0f);
        

        bento->setViewMatrix(view);
        bento->setProjectionMatrix(projection);
        bento->bindTexture(swordTex);

        for(Object* obj : objects){
            obj->draw(bento);
        }

        bento->setVertices(groundMesh->getVertexBuffer());
        bento->setNormals(groundMesh->getNormalBuffer());
        bento->setUvs(groundMesh->getUVBuffer());
        bento->bindTexture(groundTex);
 
        for(int i = -8; i < 9; i++){
            for(int j = -8; j < 9; j++){
                model = glm::scale(glm::translate(glm::mat4(1.0),glm::vec3(i*2,-1,j*2)),glm::vec3(0.005,0.005,0.005));
                bento->setModelMatrix(model);
                bento->draw();
            }
        }


        bento->imguiNewFrame();
        ImGui::Begin("aaa");

        if(showDemoWindow)ImGui::ShowDemoWindow(&showDemoWindow);
        ImGui::Text("const char* text");
        ImGui::Text("framework: %s",bento->getFramework().c_str());
        if(ImGui::Button("amnogus")){bento->exit();}
        if(ImGui::Button(showDemoWindow?"show demo? true":"show demo? false")){showDemoWindow = showDemoWindow?false:true;}


        float centerX = windowSize.x / 2.0f;
        float centerY = windowSize.y / 2.0f;
        float crosshairSize = 10.0f;
        float lineWidth = 1.5f;
        float gapSize = 4.0f;
        float dotSize = 2.0f;
        bool dot = true;
        bool lines = false;
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        if(lines){
            drawList->AddLine({ centerX - crosshairSize - gapSize, centerY }, { centerX - gapSize, centerY }, ImColor(255, 255, 255), lineWidth);
            drawList->AddLine({ centerX + gapSize, centerY }, { centerX + crosshairSize + gapSize, centerY }, ImColor(255, 255, 255), lineWidth);
            drawList->AddLine({ centerX, centerY - crosshairSize - gapSize }, { centerX, centerY - gapSize }, ImColor(255, 255, 255), lineWidth);
            drawList->AddLine({ centerX, centerY + gapSize }, { centerX, centerY + crosshairSize + gapSize }, ImColor(255, 255, 255), lineWidth);
        }
        if(dot)drawList->AddCircleFilled({centerX,centerY}, dotSize, ImColor(255, 255, 255));

        drawList->AddText({10,10},ImColor(255,255,255),(std::to_string(position.x)+" "+std::to_string(position.y)+" "+std::to_string(position.z)).c_str());
        drawList->AddText({10,30},ImColor(255,255,255),(std::to_string(direction.x)+" "+std::to_string(direction.y)+" "+std::to_string(direction.z)).c_str());
        centerX = windowSize.x-120;
        centerY = windowSize.y-120;
        drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX,centerY+100*sin(angleY-pi/2)),ImColor(255,0,0),2);
        drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX+100*cos(-angleX),centerY+100*sin(-angleX)*cos(angleY-pi/2)),ImColor(0,255,0),2);
        drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX+100*cos(-angleX+pi/2),centerY+100*sin(-angleX+pi/2)*cos(angleY-pi/2)),ImColor(0,0,255),2);
        drawList->AddEllipse({centerX,centerY},ImVec2(10,10),100*cos(angleY-pi/2),ImColor(150,150,150));
        drawList->AddCircle({centerX,centerY},100,ImColor(200,200,200));

        ImGui::End();
        bento->imguiRender();


        bento->render();

        elapsedTime+=1.0f / 70.0f;
        ticks++;
    }
    
    bento->exit();
    delete bento;
    return 0;
}


void processInputs(Bento *bento, glm::vec3 &position, glm::vec2 &lastMousePos, float &angleX, float &angleY, float speed){
    if(bento->getKey(KEY_A))position -= glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f)) * speed;
    if(bento->getKey(KEY_S))position -= glm::vec3(sin(angleX),0,cos(angleX)) * speed;
    if(bento->getKey(KEY_D))position += glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f)) * speed;
    if(bento->getKey(KEY_W))position += glm::vec3(sin(angleX),0,cos(angleX)) * speed;
    if(bento->getKey(KEY_E))position.y += 0.1;
    if(bento->getKey(KEY_Q))position.y -= 0.1;


    if(bento->getKey(KEY_L))angleX -= 0.1;
    if(bento->getKey(KEY_J))angleX += 0.1;
    if(bento->getKey(KEY_I))angleY += 0.1;
    if(bento->getKey(KEY_K))angleY -= 0.1;

    if(bento->getMouse(MOUSE_LEFT)){
        
        glm::vec2 windowSize = bento->getWindowSize();
        glm::vec2 windowPos = bento->getWindowPos();

        glm::vec2 mousePos = bento->getMousePosition();

        /*
        glm::vec2 center = floor(windowPos+glm::vec2(windowSize.x/2,windowSize.y/2));
        angleX += (center.x-mousePos.x)*0.002;
        angleY += (center.y-mousePos.y)*0.002;
        bento->setMousePosition(center);
        */
        angleX += (lastMousePos.x-mousePos.x)*0.002;
        angleY += (lastMousePos.y-mousePos.y)*0.002;
        lastMousePos = mousePos;
    }else{
        lastMousePos = bento->getMousePosition();
    }

    /*glm::vec2 pAxis = bento->getControllerAxis(0,GAMEPAD_JOYSTICK_LEFT)-leftOffset;
    bool aPressed = bento->getControllerButton(0,GAMEPAD_KEY_A);
    bool bPressed = bento->getControllerButton(0,GAMEPAD_KEY_R2);
    position += glm::vec3(
        pAxis.x*sin(angleX - 3.14f/2.0f)*speed - pAxis.y*sin(angleX)*speed,
        (aPressed?1:0)-(bPressed?1:0),
        pAxis.x*cos(angleX - 3.14f/2.0f)*speed - pAxis.y*cos(angleX)*speed
    );

    glm::vec2 cAxis = bento->getControllerAxis(0,GAMEPAD_JOYSTICK_RIGHT)-rightOffset;
    angleX -= cAxis.x*0.05;
    angleY -= cAxis.y*0.05;*///
}