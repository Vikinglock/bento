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
#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"
#include "lib/glm/gtc/type_ptr.hpp"
#include <cstring>

#include "bento.h"


int main() {
    float elapsedTime = 0.0f;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    loadOBJ("suzanne.obj",vertices,uvs,normals);

    std::vector<glm::vec3> vertices2;
    std::vector<glm::vec2> uvs2;
    std::vector<glm::vec3> normals2;
    loadOBJ("cube.obj",vertices2,uvs2,normals2);

    std::vector<glm::vec3> vertices3;
    std::vector<glm::vec2> uvs3;
    std::vector<glm::vec3> normals3;
    loadOBJ("yosh.obj",vertices3,uvs3,normals3);

    std::vector<glm::vec3> vertices4;
    std::vector<glm::vec2> uvs4;
    std::vector<glm::vec3> normals4;
    loadOBJ("ground.obj",vertices4,uvs4,normals4);

    Bento *bento = new Bento();
    bento->init("ベント",1000,1000);
    Texture *tex = new Texture("awesomeface.png");
    Texture *yosh = new Texture("yosh.png");
    Texture *tex2 = new Texture("box.png");
    Texture *groundTex = new Texture("ground.png");

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::vec2 windowSize;
    glm::vec2 windowPos;
    float angleX = 3.14, angleY = 0;
    int ticks = 0;
    bool fullscreenable = true;
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


        #ifdef USE_METAL
            if(bento->getKey(KEY_LEFT_CONTROL)&&bento->getKey(KEY_LEFT_COMMAND)&&bento->getKey(KEY_F)&&fullscreenable){
                bento->toggleFullscreen();
               fullscreenable = false;
            }
            if(!bento->getKey(KEY_F))fullscreenable = true;

            if(bento->getKey(KEY_LEFT_COMMAND)&&bento->getKey(KEY_Q)){
                bento->exit();
            }
        #endif

        //glm::vec2 center = windowPos+glm::vec2(windowSize.x/2,windowSize.y/2);
        //mousePos = bento->getMousePosition();
        //angleX += (center.x-mousePos.x)*0.001;
        //angleY += -(center.y-mousePos.y)*0.001;
        //bento->setMousePosition(center);


        direction = glm::vec3(
            cos(angleY) * sin(angleX),
            sin(angleY),
            cos(angleY) * cos(angleX)
        );
        right = glm::vec3(
            sin(angleX) * cos(0),
            sin(0),
            cos(angleX) * cos(0)
        );
        up = glm::cross( right, direction );

        view = glm::lookAt(position,position+direction,glm::vec3(0,1,0));
        projection = glm::perspective(glm::radians(45.0f), windowSize.x / windowSize.y, 0.1f, 100.0f);

        model = glm::translate(glm::mat4(1.0),glm::vec3(3,1,0));

        bento->setViewMatrix(view);
        bento->setProjectionMatrix(projection);

        bento->setVertices(vertices);
        bento->setNormals(normals);
        bento->setUvs(uvs);
        bento->setModelMatrix(model);
        bento->bindTexture(yosh,0);

        bento->draw();

        model = glm::translate(glm::mat4(1.0),glm::vec3(-3,0,0));

        bento->setViewMatrix(view);
        bento->setProjectionMatrix(projection);
        
        bento->setVertices(vertices3);
        bento->setNormals(normals3);
        bento->setUvs(uvs3);
        bento->setModelMatrix(model);
        bento->bindTexture(tex2,0);

        bento->draw();

        model = glm::mat4(1.0f);


        bento->setVertices(vertices2);
        bento->setNormals(normals2);
        bento->setUvs(uvs2);
        bento->setModelMatrix(model);
        bento->bindTexture(tex,0);

        bento->draw();
        

        bento->setVertices(vertices4);
        bento->setNormals(normals4);
        bento->setUvs(uvs4);
        bento->bindTexture(groundTex,0);

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