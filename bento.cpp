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

int main() {
    Bento *bento = new Bento();
    bento->init("ベント",1000,1000);
    float elapsedTime = 0.0f;

    Texture *epicFaceTex = new Texture("resources/awesomeface.png");
    Texture *swordTex = new Texture("resources/sword.png");
    Texture *boxTex = new Texture("resources/box.png");
    Texture *groundTex = new Texture("resources/ground.png");

    vertexBuffer suzanneVertexBuffer;//NOTE THIS IS NOT THE SAME AS THE OPENGL VBO
    normalBuffer suzanneNormalBuffer;
    uvBuffer     suzanneUVBuffer;
    vertexBuffer cubeVertexBuffer;
    normalBuffer cubeNormalBuffer;
    uvBuffer     cubeUVBuffer;
    vertexBuffer swordVertexBuffer;
    normalBuffer swordNormalBuffer;
    uvBuffer     swordUVBuffer;
    vertexBuffer groundVertexBuffer;
    normalBuffer groundNormalBuffer;
    uvBuffer     groundUVBuffer;
    std::vector<glm::vec3> suzanneVertices, suzanneNormals;
    std::vector<glm::vec2> suzanneUvs;
    loadOBJ("resources/suzanne.obj",suzanneVertices,suzanneUvs,suzanneNormals);
    std::vector<glm::vec3> cubeVertices, cubeNormals;
    std::vector<glm::vec2> cubeUvs;
    loadOBJ("resources/cube.obj",cubeVertices,cubeUvs,cubeNormals);
    std::vector<glm::vec3> swordVertices, swordNormals;
    std::vector<glm::vec2> swordUvs;
    loadOBJ("resources/sword.obj",swordVertices,swordUvs,swordNormals);
    std::vector<glm::vec3> groundVertices, groundNormals;
    std::vector<glm::vec2> groundUvs;
    loadOBJ("resources/ground.obj",groundVertices,groundUvs,groundNormals);
    suzanneVertexBuffer.setBuffer(suzanneVertices);
    suzanneNormalBuffer.setBuffer(suzanneNormals);
    suzanneUVBuffer.    setBuffer(suzanneUvs);
    cubeVertexBuffer.setBuffer(cubeVertices);
    cubeNormalBuffer.setBuffer(cubeNormals);
    cubeUVBuffer.    setBuffer(cubeUvs);
    swordVertexBuffer.setBuffer(swordVertices);
    swordNormalBuffer.setBuffer(swordNormals);
    swordUVBuffer.    setBuffer(swordUvs);
    groundVertexBuffer.setBuffer(groundVertices);
    groundNormalBuffer.setBuffer(groundNormals);
    groundUVBuffer.setBuffer(groundUvs);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::vec2 windowSize;
    glm::vec2 windowPos;
    float angleX = 3.14, angleY = 0;
    int ticks = 0;
    glm::mat4 projection = glm::mat4(1.0f);
    glm::vec3 position(0.0f,1.0f,3.0f);
    glm::vec3 direction(0.0f,0.0f,-1.0f), right(1,0,0), up(0,1,0);
    glm::vec2 displaySize = bento->getDisplaySize();
    glm::vec2 mousePos = bento->getMousePosition();
    bento->setMousePosition(glm::vec2(500,500));
    while (bento->isRunning()){
        bento->predraw();
        windowSize = bento->getWindowSize();
        windowPos = bento->getWindowPos();
        float speed = 0.1;
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
            glm::vec2 center = windowPos+glm::vec2(windowSize.x/2,windowSize.y/2);
            mousePos = bento->getMousePosition();
            angleX += (center.x-mousePos.x)*0.001;
            angleY += -(center.y-mousePos.y)*0.001;
            bento->setMousePosition(center);
        }


        direction = glm::vec3(
            cos(angleY) * sin(angleX),
            sin(angleY),
            cos(angleY) * cos(angleX)
        );
        right = glm::vec3(
            sin(angleX - 3.14f/2.0f),
            0,
            cos(angleX - 3.14f/2.0f)
        );
        up = glm::cross( right, direction );
        view = glm::lookAt(position,position+direction,up);
        projection = glm::perspective(glm::radians(45.0f), windowSize.x / windowSize.y, 0.1f, 100.0f);

        model = glm::translate(glm::mat4(1.0),glm::vec3(3,1,0));

        bento->setViewMatrix(view);
        bento->setProjectionMatrix(projection);

        bento->setVertices(suzanneVertexBuffer);
        bento->setNormals(suzanneNormalBuffer);
        bento->setUvs(suzanneUVBuffer);
        bento->setModelMatrix(model);
        bento->bindTexture(boxTex);

        bento->draw();

        model = glm::translate(glm::mat4(1.0),glm::vec3(-3,0,0));

        bento->setViewMatrix(view);
        bento->setProjectionMatrix(projection);
        
        bento->setVertices(swordVertexBuffer);
        bento->setNormals(swordNormalBuffer);
        bento->setUvs(swordUVBuffer);
        bento->setModelMatrix(model);
        bento->bindTexture(swordTex);

        bento->draw();

        model = glm::mat4(1.0f);


        bento->setVertices(cubeVertexBuffer);
        bento->setNormals(cubeNormalBuffer);
        bento->setUvs(cubeUVBuffer);
        bento->setModelMatrix(model);
        bento->bindTexture(epicFaceTex);

        bento->draw();
        
        bento->setVertices(groundVertexBuffer);
        bento->setNormals(groundNormalBuffer);
        bento->setUvs(groundUVBuffer);
        bento->bindTexture(groundTex);

        for(int i = -9; i < 10; i++){
            for(int j = -9; j < 10; j++){
                model = glm::scale(glm::translate(glm::mat4(1.0),glm::vec3(i*2,-1,j*2)),glm::vec3(0.005,0.005,0.005));
                bento->setModelMatrix(model);
                bento->draw();
            }
        }

        bento->render();

        elapsedTime+=1.0f / 70.0f;
        ticks++;
    }
    
    bento->exit();
    delete bento;
    return 0;
}