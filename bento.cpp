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

#include <chrono>

#include "bento/bento.h"

glm::vec3 position(0.0f,1.0f,3.0f), speed(0,0,0), maxSpeed(0,0,0);
glm::vec3 direction(0.0f,0.0f,-1.0f), right(1,0,0), up(0,1,0);

std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
float deltaTime = 0.0f, elapsedTime = 0.0f;

int main() {
    Bento *bento = new Bento();
    bento->init("ベント",1000,1000);
    bento->focus();

    bento->setClearColor(glm::vec4(0,0,0,1));

    bento->initImgui();

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    Texture* scrnTex;

    Texture* tex = new Texture("./resources/grass.png");

    
    bento->addLight(glm::vec3(10,1,10),glm::vec3(1,1,1),glm::vec3(1,1,1),glm::vec3(1,1,1),1.0,0.9,0.2);
    bento->addLight(glm::vec3(10,3,1),glm::vec3(10,10,10),glm::vec3(1,1,1),glm::vec3(1,1,1),1.0,0.9,0.2);

    std::vector<Object*> objects;
    //objects.emplace_back(new Object("sword",glm::vec3(0,0,0),"resources/sword.obj"));


    for(int i = -25; i < 25; i+=2)
        for(int j = -25; j < 25; j+=2)
            objects.emplace_back(new Object("sword",glm::vec3(i,0,j),"resources/sword.obj"));

    Texture *swordTex = new Texture("./resources/sword.png");//or you can do "resources/sword.png"

    Mesh *groundMesh = new Mesh("./resources/ground.obj");
    Texture *groundTex = new Texture("./resources/ground.png");


    objects.emplace_back(new Object("shphere",glm::vec3(0,10,0),"resources/hmm.obj","resources/hmm.png"));
    
    float angleX = 3.14, angleY = 0;
    
    while (bento->isRunning()){
        glm::vec2 windowSize(bento->getWindowSize());

        direction = glm::vec3(cos(angleY) * sin(angleX),sin(angleY),cos(angleY) * cos(angleX));
        right = glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f));
        up = glm::cross( right, direction );
        float tilt = 0.003;
        view = glm::lookAt(position,position+direction,up+glm::vec3(speed.x*tilt,0,speed.z*tilt));


        glm::vec2 viewSize(windowSize*glm::vec2(0.5));

        bento->predrawTex(viewSize.x,viewSize.y);

        projection = glm::perspective(glm::radians(110.0f), fmax(windowSize.x,1.0f) / fmax(windowSize.y,1.0f), 0.01f, 1000.0f);

        bento->setViewMatrix(view,position);
        bento->setProjectionMatrix(projection);
        bento->bindTexture(swordTex);
        for(Object* obj : objects)obj->drawTex(bento);

        bento->setVertices(groundMesh->getVertexBuffer());
        bento->setNormals(groundMesh->getNormalBuffer());
        bento->setUvs(groundMesh->getUVBuffer());
        bento->bindTexture(groundTex);
        model = glm::translate(glm::mat4(1.0),glm::vec3(0.0,0.1,0.0));
        bento->setModelMatrix(model);
        bento->drawTex();


        scrnTex = bento->renderTex();
        
        bento->predraw();

        bento->setVertices(groundMesh->getVertexBuffer());
        bento->setNormals(groundMesh->getNormalBuffer());
        bento->setUvs(groundMesh->getUVBuffer());
        bento->bindTexture(scrnTex);
        bento->draw();


        bento->imguiNewFrame();
        ImGui::Begin("NOTICE");
        ImGui::Text("PLACEHOLDER");
        ImGui::Image((ImTextureID)scrnTex->getTexture(),ImVec2(viewSize.x,viewSize.y));


        if(bento->getKey(KEY_A))position -= glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f))*glm::vec3(0.3);
        if(bento->getKey(KEY_S))position -= glm::vec3(sin(angleX),0,cos(angleX))*glm::vec3(0.3);
        if(bento->getKey(KEY_D))position += glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f))*glm::vec3(0.3);
        if(bento->getKey(KEY_W))position += glm::vec3(sin(angleX),0,cos(angleX))*glm::vec3(0.3);
        if(bento->getKey(KEY_E))position.y += 0.1;
        if(bento->getKey(KEY_Q))position.y -= 0.1;


    
        if(bento->getKey(KEY_L))angleX -= 0.1;
        if(bento->getKey(KEY_J))angleX += 0.1;
        if(bento->getKey(KEY_I))angleY += 0.1;
        if(bento->getKey(KEY_K))angleY -= 0.1;



        float centerX = windowSize.x / 2.0f;
        float centerY = windowSize.y / 2.0f;
        float crosshairSize = 10.0f;
        float lineWidth = 1.5f;
        float gapSize = 4.0f;
        float dotSize = 2.0f;
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        if(bento->getKey(KEY_F)){
            drawList->AddText({10,10},ImColor(255,255,255),(std::to_string(position.x)+" "+std::to_string(position.y)+" "+std::to_string(position.z)).c_str());
            drawList->AddText({10,30},ImColor(255,255,255),(std::to_string(direction.x)+" "+std::to_string(direction.y)+" "+std::to_string(direction.z)).c_str());
            drawList->AddText({10,50},ImColor(255,255,255),(std::to_string(speed.x)+" "+std::to_string(speed.y)+" "+std::to_string(speed.z)).c_str());
            drawList->AddText({10,70},ImColor(255,255,255),(std::to_string(sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))).c_str());
            drawList->AddText({10,90},ImColor(255,255,255),(std::to_string(sqrt(speed.x*speed.x+speed.z*speed.z))).c_str());
            drawList->AddText({10,120},ImColor(255,255,255),("FPS: "+std::to_string(1/deltaTime)).c_str());
            drawList->AddText({10,150},ImColor(255,255,255),std::to_string((int)(elapsedTime*2)).c_str());
            drawList->AddText({10,200},ImColor(255,255,255),std::to_string(5-(position-glm::vec3(0)).length()*0.5).c_str());
            centerX = windowSize.x-120;
            centerY = windowSize.y-120;
            drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX,centerY+100*sin(angleY-pi/2)),ImColor(255,0,0),2);
            drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX+100*cos(-angleX),centerY+100*sin(-angleX)*cos(angleY-pi/2)),ImColor(0,255,0),2);
            drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX+100*cos(-angleX+pi/2),centerY+100*sin(-angleX+pi/2)*cos(angleY-pi/2)),ImColor(0,0,255),2);
            drawList->AddEllipse({centerX,centerY},ImVec2(10,10),100*cos(angleY-pi/2),ImColor(150,150,150));
            drawList->AddCircle({centerX,centerY},100,ImColor(200,200,200));
        }

        ImGui::End();


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