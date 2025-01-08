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

/*

THIS IS AN EXAMPLE

this is an example that can be built that can be used freely or as a model
                               ^
                        (mac)   sh run.sh -metal bento bento.cpp
                        (mac)   sh run.sh -opengl bento bento.cpp
                        (linux) sh runlinux.sh bento bento.cpp
*/

void processInputs(Bento *bento);

glm::vec2 rightOffset, leftOffset;

bool grounded = true, jumping = false, lock = true;

float playerHeight = 1.0;

float tilt = 0.0;

float fov = 100.0, fovTo = 100.0, fovDef = 100.0;

glm::vec3 position(0.0f,1.0f,3.0f), speed(0,0,0), maxSpeed(0,0,0);
glm::vec3 direction(0.0f,0.0f,-1.0f), right(1,0,0), up(0,1,0);

glm::vec2 mousePos, lastMousePos;

float angleX = 3.14, angleY = 0;

std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();;
float deltaTime = 0.0f;

Sound* sound1;
Sound* sound2;
Sound* sound3;


float elapsedTime = 0.0f, stepCounter = 0.0f;

int main() {
    Bento *bento = new Bento();
    bento->init("ベント",1000,1000);
    bento->focus();

    bento->setClearColor(glm::vec4(0,0,0,1));//automatically 0 so you don't have to set this unless you want another color

    bento->initImgui();

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
    


    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    int ticks = 0;
    
    glm::vec2 displaySize = bento->getDisplaySize();
    leftOffset = bento->getControllerAxis(0,GAMEPAD_JOYSTICK_LEFT);
    rightOffset = bento->getControllerAxis(0,GAMEPAD_JOYSTICK_RIGHT);
    glm::vec2 windowSize;
    bento->setMousePosition(glm::vec2(500,500));
    mousePos = bento->getMousePosition();
    lastMousePos = bento->getMousePosition();

    //dear imgui

    bool showDemoWindow = true;

    //audio


    bento->initSound();

    Sound* music = new Sound("./resources/space.wav");//from my other other game (fine to use also)

    music->setGain(0.25);
    music->setLoop(true);
    //music->play();


    sound1 = new Sound("./resources/step1.wav");//from my other game (free to use btw)
    sound3 = new Sound("./resources/step4.wav");//note it's .wav only
    sound2 = new Sound("./resources/step3.wav");

    sound1->setGain(0.5);
    sound2->setGain(0.5);
    sound3->setGain(0.5);


    while (bento->isRunning()){

        processInputs(bento);

        direction = glm::vec3(cos(angleY) * sin(angleX),sin(angleY),cos(angleY) * cos(angleX));
        right = glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f));
        up = glm::cross( right, direction );
        tilt = 0.003;
        view = glm::lookAt(position,position+direction,up+glm::vec3(speed.x*tilt,0,speed.z*tilt));
        windowSize = bento->getWindowSize();
        projection = glm::perspective(glm::radians(fov), fmax(windowSize.x,1.0f) / fmax(windowSize.y,1.0f), 0.01f, 1000.0f);

        fov += (fovTo-fov)/15;
        
        bento->setLightPos(0,position);


        bento->predraw();

        bento->setViewMatrix(view,position);
        bento->setProjectionMatrix(projection);

        bento->bindTexture(swordTex);//because loading this texture for every single sword would be very inefficient
        //if you really care about memory or have a pixel art tileset then i recommend looking into texture atlases (putting every sprite into a giant texture)
        //also with this approach you can offset the uv for some easy animation

        for(Object* obj : objects){//draws everything in objects (they're all swords so binding the texture before this loop would make every object have a sword texture)
            obj->draw(bento);
        }

        bento->setVertices(groundMesh->getVertexBuffer());
        bento->setNormals(groundMesh->getNormalBuffer());
        bento->setUvs(groundMesh->getUVBuffer());
        bento->bindTexture(groundTex);
        model = glm::translate(glm::mat4(1.0),glm::vec3(0.0,0.1,0.0));
        bento->setModelMatrix(model);
        bento->draw();

        bento->imguiNewFrame();


        ImGui::Begin("aaa");

        if(showDemoWindow)ImGui::ShowDemoWindow(&showDemoWindow);
        ImGui::Text("const char* text");
        ImGui::Text("framework: %s",bento->getFramework().c_str());
        ImGui::Text("operating system: %s",bento->getOperatingSystem().c_str());
        if(ImGui::Button("exit")){bento->exit();}
        if(ImGui::Button(showDemoWindow?"show demo? true":"show demo? false")){showDemoWindow = showDemoWindow?false:true;}


        float centerX = windowSize.x / 2.0f;
        float centerY = windowSize.y / 2.0f;
        float crosshairSize = 10.0f;
        float lineWidth = 1.5f;
        float gapSize = 4.0f;
        float dotSize = 2.0f;
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        drawList->AddCircleFilled({centerX,centerY}, dotSize, ImColor(255, 255, 255));
        drawList->AddText({10,10},ImColor(255,255,255),(std::to_string(position.x)+" "+std::to_string(position.y)+" "+std::to_string(position.z)).c_str());
        drawList->AddText({10,30},ImColor(255,255,255),(std::to_string(direction.x)+" "+std::to_string(direction.y)+" "+std::to_string(direction.z)).c_str());
        drawList->AddText({10,50},ImColor(255,255,255),(std::to_string(speed.x)+" "+std::to_string(speed.y)+" "+std::to_string(speed.z)).c_str());
        drawList->AddText({10,70},ImColor(255,255,255),(std::to_string(sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))).c_str());
        drawList->AddText({10,90},ImColor(255,255,255),(std::to_string(sqrt(speed.x*speed.x+speed.z*speed.z))).c_str());
        drawList->AddText({10,120},ImColor(255,255,255),("FPS: "+std::to_string(1/deltaTime)).c_str());
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
        stepCounter+=sqrt(speed.x*speed.x+speed.z*speed.z)/200.0f;

        auto currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        ticks++;
    }
    bento->exit();
    delete bento;
    return 0;
}


void processInputs(Bento *bento){
    //0.01/9.81
    speed.y -= 0.1;
    if(position.y < playerHeight)position.y = playerHeight;
    if(position.y <= playerHeight) {
        speed.y = 0.0;
        if(playerHeight>4){
            speed.x *= 0.7;
            speed.z *= 0.7;
        }else{
            speed.x *= 0.7;
            speed.z *= 0.7;
        }
        grounded = true;
        maxSpeed = speed;
    }else{
        speed.x *= 0.975;
        speed.z *= 0.975;
        grounded = false;
    }

    if(grounded && sqrt(speed.x*speed.x+speed.z*speed.z) > 0.1 && std::fmod(stepCounter, 1.0f)>0.75){
        int r = rand()%75;
        stepCounter = 0.0f;
        Sound* sound;
        if(r < 25) sound = sound1;
        else if(r < 50) sound = sound2;
        else sound = sound3;
        
        sound->play();
        int p = rand();
        sound->setPitch(std::fmod(p/1000.0,1.0f)*0.5+0.5);

    }



    float moveSpeed = 2.0;
    float jumpHeight = 5.0;

    bool running = false;

    if(bento->getKey(KEY_LEFT_SHIFT)){running = true;moveSpeed = 3.5;}

    if(grounded && sqrt(speed.x*speed.x+speed.z*speed.z) < 0.1){
        running = false;
    }

    if(bento->getMouse(MOUSE_RIGHT)){
        bento->setMouseCursor(true,0);
        lock = true;
        if(running)fovTo =  60.0;
        else fovTo =  50.0;
    }else{
        if(running)fovTo =  110.0;
        else fovTo = fovDef;
    }

    if(bento->getKey(KEY_LEFT_CONTROL)){playerHeight = 0.25;moveSpeed = 0.5;}else playerHeight = 1.0;

    if(!grounded)moveSpeed /= 10;

    if(bento->getKey(KEY_A))speed -= glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f)) * moveSpeed;
    if(bento->getKey(KEY_S))speed -= glm::vec3(sin(angleX),0,cos(angleX)) * moveSpeed;
    if(bento->getKey(KEY_D))speed += glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f)) * moveSpeed;
    if(bento->getKey(KEY_W))speed += glm::vec3(sin(angleX),0,cos(angleX)) * moveSpeed;
    if(bento->getKey(KEY_E))position.y += 0.1;
    if(bento->getKey(KEY_Q))position.y -= 0.1;


    if(bento->getKey(KEY_SPACE) && !jumping && grounded){speed.y += jumpHeight;jumping = true;grounded = false;}else{jumping = false;}

    position += speed * deltaTime;
   
    if(bento->getKey(KEY_L))angleX -= 0.1;
    if(bento->getKey(KEY_J))angleX += 0.1;
    if(bento->getKey(KEY_I))angleY += 0.1;
    if(bento->getKey(KEY_K))angleY -= 0.1;

    fovDef += bento->getScroll(0);

    if(bento->getKey(KEY_ESCAPE)){
        lock = false;
        bento->setMouseCursor(false,0);
    }

    if(lock){
        
        glm::vec2 windowSize = bento->getWindowSize();
        glm::vec2 windowPos = bento->getWindowPos();

        glm::vec2 mousePos = bento->getMousePosition();

        
        glm::vec2 center = floor(windowPos+glm::vec2(windowSize.x/2,windowSize.y/2));
        angleX += (center.x-mousePos.x)*0.002;
        angleY += (center.y-mousePos.y)*0.002;
        bento->setMousePosition(center);

        //angleX += (lastMousePos.x-mousePos.x)*0.002;
        //angleY += (lastMousePos.y-mousePos.y)*0.002;
        lastMousePos = mousePos;
    }else{
        lastMousePos = bento->getMousePosition();
    }

    /*glm::vec2 pAxis = bento->getControllerAxis(0,GAMEPAD_JOYSTICK_LEFT)-leftOffset;
    bool aPressed = bento->getControllerButton(0,GAMEPAD_KEY_A);
    bool bPressed = bento->getControllerButton(0,GAMEPAD_KEY_R2);
    position += glm::vec3(
        pAxis.x*sin(angleX - 3.14f/2.0f)*moveSpeed - pAxis.y*sin(angleX)*moveSpeed,
        (aPressed?1:0)-(bPressed?1:0),
        pAxis.x*cos(angleX - 3.14f/2.0f)*moveSpeed - pAxis.y*cos(angleX)*moveSpeed
    );

    glm::vec2 cAxis = bento->getControllerAxis(0,GAMEPAD_JOYSTICK_RIGHT)-rightOffset;
    angleX -= cAxis.x*0.05;
    angleY -= cAxis.y*0.05;*///
}