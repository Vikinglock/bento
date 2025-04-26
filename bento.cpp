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

#include "bento/lib/btBulletDynamicsCommon.h"
#include "bento/lib/BulletCollision/Gimpact/btGImpactShape.h"
#include "bento/lib/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"

#include <chrono>

#include "bento/bento.h"


//example so i won't put much effort into organization and stuff
//just know this is to see what you can do with this and how
//this is not to see how to organize your code or implement mechanics
//also it's probably recommended to separate your code into different files


glm::vec3 position(0.0f,1.0f,3.0f), speed(0,0,0), maxSpeed(0,0,0);
glm::vec3 direction(0.0f,0.0f,-1.0f), right(1,0,0), up(0,1,0);

std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
float deltaTime = 0.0f, elapsedTime = 0.0f;

glm::vec2 imguiVec2ToGlm(ImVec2 v){return glm::vec2(v.x,v.y);}
ImVec2 glmVec2ToImgui(glm::vec2 v){return ImVec2(v.x,v.y);}
int min(int a,int b){return a>b?b:a;}
int max(int a,int b){return a>b?a:b;}
float clamp(int num,int m,int x){return max(min(num,x),m);}
float fclamp(float num,float m,float x){return fmax(fmin(num,x),m);}

struct Light
{
    Light(std::string n,int i,glm::vec3 p,glm::vec3 a=glm::vec3(1.0f), glm::vec3 d=glm::vec3(1.0f), glm::vec3 s=glm::vec3(1.0f), float c =1.f,float l=0.09f,float q=0.032f):name(n),position(p),ambient(a),diffuse(d),specular(s),constant(c),linear(l),quadratic(q),index(i){}
    void update(Bento* bento){
        bento->setUniform("positions",position,index*sizeof(glm::vec3));
        bento->setUniform("constants",constant,index*sizeof(float));
        bento->setUniform("linears",linear,index*sizeof(float));
        bento->setUniform("quadratics",quadratic,index*sizeof(float));
        bento->setUniform("ambients",ambient,index*sizeof(glm::vec3));
        bento->setUniform("diffuses",diffuse,index*sizeof(glm::vec3));
        bento->setUniform("speculars",specular,index*sizeof(glm::vec3));
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

void updateLights(std::vector<Light> lights,Bento* bento,float tPDist);

glm::vec3 positions[50];
float constants[50];
float linears[50];
float quadratics[50];
glm::vec3 ambients[50];
glm::vec3 diffuses[50];
glm::vec3 speculars[50];
glm::vec3 ambientColor(142.0/255.0,169.0/255.0,168.0/255.0);

int main() {
    Bento* bento = new Bento();
    bento->init("ベント",1000,1000);
    bento->focus();
    bento->enable(depthTest);

    Shader* defaultShader = new Shader("./resources/shaders/shader.vert","./resources/shaders/shader.frag");
    Shader* screenShader = new Shader("./resources/shaders/screen.vert","./resources/shaders/screen.frag");
    Shader* thresholdShader = new Shader("./resources/shaders/threshold.vert","./resources/shaders/threshold.frag");
    Shader* boxBlurShader = new Shader("./resources/shaders/blur.vert","./resources/shaders/blur.frag");
    Shader* combineShader = new Shader("./resources/shaders/combine.vert","./resources/shaders/combine.frag");


    bento->setClearColor(glm::vec4(142.0/255.0,169.0/255.0,168.0/255.0,1));

    bento->initImgui();

    ImFont* ogfont = ImGui::GetFont();
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF("./resources/fonts/notoSansMono/NotoSansMono-Regular.ttf",15);
    ImFont* boldFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("./resources/fonts/notoSansMono/NotoSansMono-Bold.ttf",15);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

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

    Texture* scrnTex;
    Texture* blurTex;
    Texture* colTex;
    Texture* nrmlTex;
    Texture* depthTex;
    Texture* posTex;

    Texture* tex = new Texture("./resources/grass.png");
    

    std::vector<Light> lights;

    lights.emplace_back("light1",lights.size(),glm::vec3(10,1,10),glm::vec3(1,1,1),glm::vec3(1,1,1),glm::vec3(1,1,1),1.0,0.9,0.2);
    lights.emplace_back("light2",lights.size(),glm::vec3(10,3,1),glm::vec3(10,10,10),glm::vec3(1,1,1),glm::vec3(1,1,1),1.0,0.9,0.2);
    lights.emplace_back("light3",lights.size(),glm::vec3(1,1,10),glm::vec3(1,1,1),glm::vec3(1,0,0),glm::vec3(1,0.6,0.6),0.1,0.8,0.01);

    std::vector<Object*> objects;

    objects.emplace_back(new Object("shphere",glm::mat4(1.0),"resources/hmm.obj","resources/hmm.png"));
    objects.emplace_back(new Object("shphere",glm::mat4(1.0),"resources/rayHit.obj","resources/hmm.png"));
    objects.emplace_back(new Object("shphere",glm::mat4(1.0),"resources/scarypng.obj","resources/horse.png"));
    objects.emplace_back(new Object("player",glm::mat4(1.0),"resources/cube.obj","resources/hmm.png"));

    Texture *hmTex = new Texture("./resources/hmm.png");
    Mesh *icoMesh = new Mesh("./resources/hmm.obj");
    for(int i = 0; i < 51; i++)objects.emplace_back(new Object("shphere",glm::mat4(1.0),icoMesh,hmTex));


    Texture *cubeTex = new Texture("./resources/gradient.png");
    Mesh *cubeMesh = new Mesh("./resources/cube.obj");
    for(int i = 0; i < 501; i++)objects.emplace_back(new Object("cube",glm::mat4(1.0),cubeMesh,cubeTex));


    Texture *swordTex = new Texture("./resources/sword.png");//i assume you know the least bit about c++ and file paths
    Mesh *swordMesh = new Mesh("./resources/sword.obj");
    for(int i = -25; i < 25; i+=2)
        for(int j = 0; j < 25; j+=2)
            objects.emplace_back(new Object("sword",glm::translate(glm::mat4(1.0),glm::vec3(i,j,-10)),swordMesh,swordTex));

    Mesh *groundMesh = new Mesh("./resources/ground.obj");
    Texture *groundTex = new Texture("./resources/ground.png");

    Mesh *handgun = new Mesh("./resources/scarypng.obj");
    Texture *handgunTex = new Texture("./resources/handgun.png");

    std::vector<glm::vec3> vs, ns;
    std::vector<glm::vec2> us;
    loadOBJ("./resources/suzanne.obj",vs,us,ns);
    std::vector<float> vo;
    for(glm::vec3 v:vs)vo.push_back(v.x);



    float angleX = 3.14, angleY = 0;
    float fov = 110.0f, tPDist = 10.0f, tPDistSensitivity = 0.75f;
    int pov = 0;

    glm::vec2 mousePos(0,0), lastMousePos(0,0);

    glm::vec2 windowSize(bento->getWindowSize());
    glm::vec2 windowPos(bento->getWindowPos());
    glm::vec2 viewSize(windowSize);
    glm::vec2 viewPos(0,0);

    glm::vec2 mouseSpaceSize(windowSize);
    glm::vec2 mouseSpacePos(0,0);

    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);
    btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver();
    btDiscreteDynamicsWorld* world = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    btVector3 gravity(0,-9.8,0);
    world->setGravity(gravity);

    btRigidBody* boxes[552];
    btVector3 planeNormal(0, 1, 0);
    btScalar planeDistance = 0.0f;
    btStaticPlaneShape* groundShape = new btStaticPlaneShape(planeNormal, planeDistance);
    btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), btVector3(0,0,0)));
    btScalar mass = 0.f;
    btVector3 groundInertia(0, 0, 0);
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(mass, groundMotionState, groundShape, groundInertia);
    btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
    world->addRigidBody(groundRigidBody);


    btTriangleMesh* mesh = new btTriangleMesh();

    std::vector<glm::vec3> vertices,normals;
    std::vector<glm::vec2> uvs;
    loadOBJ("./resources/icosahedron.obj",vertices,uvs,normals);//or you can use a btSphereShape like a normal person

    for(int i = 0; i < 51; i++){
        btTriangleMesh* mesh = new btTriangleMesh();
        for (int j = 0; j < vertices.size(); j += 3) {
            btVector3 v0(vertices[j].x,   vertices[j].y,   vertices[j].z);
            btVector3 v1(vertices[j+1].x, vertices[j+1].y, vertices[j+1].z);
            btVector3 v2(vertices[j+2].x, vertices[j+2].y, vertices[j+2].z);
            mesh->addTriangle(v0, v1, v2);
        }
    
        btGImpactMeshShape* gImpactShape = new btGImpactMeshShape(mesh);
        gImpactShape->updateBound();
    
        btVector3 inertia(0, 0, 0);
        gImpactShape->calculateLocalInertia(1000.0, inertia);
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(0+i*1.5,8+i,0));
        btRigidBody::btRigidBodyConstructionInfo rbInfo(1000.0, new btDefaultMotionState(transform), gImpactShape, inertia);
        boxes[i] = new btRigidBody(rbInfo);
        world->addRigidBody(boxes[i]);
    }

    for (int i = 51; i < 552; i++) {
        btBoxShape* cubeShape = new btBoxShape(btVector3(1,1,1));
        btVector3 inertia(0, 0, 0);
        cubeShape->calculateLocalInertia(1000.0, inertia);
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(0+i*1.5, 8+i, 0));
        btRigidBody::btRigidBodyConstructionInfo rbInfo(1000.0, new btDefaultMotionState(transform), cubeShape, inertia);
        boxes[i] = new btRigidBody(rbInfo);
        world->addRigidBody(boxes[i]);
    }


    btCollisionShape* playerShape = new btCapsuleShape(0.5f,5.0f);

    btTransform playerTransform;
    playerTransform.setIdentity();
    playerTransform.setOrigin(btVector3(0, 1, -10));
    btVector3 playerInertia(0,0,0);
    playerShape->calculateLocalInertia(80.0f,playerInertia);
    btRigidBody::btRigidBodyConstructionInfo playerBodyCI(80.0f, new btDefaultMotionState(playerTransform), playerShape, playerInertia);
    btRigidBody* playerBody = new btRigidBody(playerBodyCI);
    world->addRigidBody(playerBody);
    btVector3 angularFactor(0.0f, 1.0f, 0.0f);
    playerBody->setAngularFactor(angularFactor);
    
    while (bento->isRunning()){
        windowSize = bento->getWindowSize();
        windowPos = bento->getWindowPos();
        mousePos = bento->getMousePosition();


        float lastAngleX = angleX;
        float lastAngleY = angleY;
        glm::vec3 lastSpeed(0,0,0);
        static bool rightClicked = false;
        static bool leftClicked = false;
        bool lastLeftClicked = leftClicked;

        static bool fpsController = false;
        static bool grounded = false;

        btTransform trans;
        playerBody->getMotionState()->getWorldTransform(trans);
        trans.getOpenGLMatrix(glm::value_ptr(objects[3]->transformation));
        playerBody->activate();
        if(fpsController){
            btVector3 speedBT = playerBody->getLinearVelocity();
            lastSpeed = speed;
            speed = glm::vec3(speedBT.x(),speedBT.y(),speedBT.z());
            btVector3 start = playerBody->getWorldTransform().getOrigin();
            btVector3 end = start + btVector3(0,-3.5,0);
            btCollisionWorld::ClosestRayResultCallback rayCallback(start, end);
            world->rayTest(start, end, rayCallback);
            grounded = rayCallback.hasHit();

            if(!grounded){
                speed.y -= 1;
            }else{
                if(!bento->getKey(KEY_SPACE)){
                    speed.x *= 0.9;
                    speed.z *= 0.9;
                }
            }

            btVector3 p = trans.getOrigin();
            position = glm::vec3(p.x(),p.y(),p.z());
        }
        static bool viewportHover = false;
        static bool viewportFocus = false;
        static bool middleMouseToLook = true;
        static bool rightMouseToLook = false;
        static bool clickLookToFocus = true;

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

        static bool invTurnX = true;
        static float turnSensX = 0.005f;
        static bool invTurnY = true;
        static float turnSensY = 0.005f;

        static bool turnUntilEscape = false;

        if(viewportFocus){
            static float moveSpeed;
            if(shiftToGoFast&&bento->getKey(KEY_LEFT_SHIFT))moveSpeed = fastSpeed;else moveSpeed = slowSpeed;

            if(wasdToMove){
                if(fpsController){
                    if(grounded){
                        if(bento->getKey(KEY_SPACE))speed.y = 25.0;
                    }
                    glm::vec3 move(0,0,0);
                    if(bento->getKey(KEY_A))move -= glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f));
                    if(bento->getKey(KEY_S))move -= glm::vec3(sin(angleX),0,cos(angleX));
                    if(bento->getKey(KEY_D))move += glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f));
                    if(bento->getKey(KEY_W))move += glm::vec3(sin(angleX),0,cos(angleX));
                    double currentHSpeed = sqrt(speed.x*speed.x+speed.z*speed.z);
                    glm::vec3 pSpeed = speed+move;
                    if((currentHSpeed<250*moveSpeed||sqrt(pSpeed.x*pSpeed.x+pSpeed.z*pSpeed.z)<currentHSpeed) && glm::length(move)>0)speed += glm::normalize(move)*moveSpeed*25.0f;
                }else{
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
            }

            if(ijklAsLookKeys){
                if(bento->getKey(KEY_L))angleX -= 0.1;
                if(bento->getKey(KEY_J))angleX += 0.1;
                if(bento->getKey(KEY_I))angleY += 0.1;
                if(bento->getKey(KEY_K))angleY -= 0.1;
            }

            static bool lookClicked = false;
            static bool waitForLookRelease = false;
            static bool escFocused = false;

            int lookKey = MOUSE_MIDDLE;
            if(rightMouseToLook)lookKey = MOUSE_RIGHT;

            if(turnUntilEscape&&bento->getMouse(lookKey)){
                escFocused = true;
            }
            if(turnUntilEscape&&bento->getKey(KEY_ESCAPE)){
                escFocused = false;
            }
            if(bento->getMouse(lookKey)||(turnUntilEscape&&escFocused)){
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
                        angleX += (lastMousePos.x-mousePos.x)*(invTurnX?1:-1)*turnSensX;
                        angleY += (lastMousePos.y-mousePos.y)*(invTurnY?1:-1)*turnSensY;
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

        {
            glm::vec3 forward = glm::normalize(position-glm::vec3(0,10,0));
            glm::vec3 right   = glm::normalize(glm::cross(glm::vec3(0,1,0), forward));
            glm::vec3 actualUp = glm::cross(forward, right);
            glm::mat4 rotation = glm::mat4(1.0f);
            rotation[0] = glm::vec4(right, 0.0f);
            rotation[1] = glm::vec4(actualUp, 0.0f);
            rotation[2] = glm::vec4(forward, 0.0f);
            glm::mat4 translation = glm::translate(glm::mat4(1.0f),glm::vec3(0,10,0));
            objects[0]->transformation = translation * rotation * glm::rotate(glm::mat4(1.0f),-glm::half_pi<float>(),glm::vec3(0,1,0));
        }

        static btRigidBody* moveBody;
        static btRigidBody* hitBody;
        static glm::vec3 hitPoint(0,0,0);

        glm::vec4 rayClip = glm::vec4(2*((mousePos.x-windowPos.x-mouseSpacePos.x) / mouseSpaceSize.x)-1,-2*((mousePos.y-windowPos.y-mouseSpacePos.y) / mouseSpaceSize.y)+1, -1.0f, 1.0f);
        glm::vec4 rayEye = glm::inverse(projection) * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
        glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));
        btVector3 rayFrom;
        if(pov==0)rayFrom = btVector3((position-direction*tPDist).x,(position-direction*tPDist).y,(position-direction*tPDist).z)+btVector3(rayWorld.x, rayWorld.y, rayWorld.z)*2.0f;
        else rayFrom = btVector3(position.x,position.y,position.z)+btVector3(rayWorld.x, rayWorld.y, rayWorld.z)*2.0f;
        btVector3 rayTo = rayFrom + btVector3(rayWorld.x, rayWorld.y, rayWorld.z) * 1000.0f;
        btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom, rayTo);
        world->rayTest(rayFrom, rayTo, rayCallback);
        if (rayCallback.hasHit()) {
            hitBody = (btRigidBody*)btRigidBody::upcast(rayCallback.m_collisionObject);
            hitPoint = glm::vec3(rayCallback.m_hitPointWorld.x(), rayCallback.m_hitPointWorld.y(), rayCallback.m_hitPointWorld.z());
        }

        if(moveBody){
            btTransform transform = moveBody->getWorldTransform();
            if(pov==0)transform.setOrigin(btVector3(((position-direction*tPDist)+direction*glm::vec3(5)).x,((position-direction*tPDist)+direction*glm::vec3(5)).y,((position-direction*tPDist)+direction*glm::vec3(5)).z));
            else transform.setOrigin(btVector3((position+direction*glm::vec3(5)).x,(position+direction*glm::vec3(5)).y,(position+direction*glm::vec3(5)).z));
            moveBody->setWorldTransform(transform);
            if (moveBody->getMotionState()) {
                moveBody->getMotionState()->setWorldTransform(transform);
            }
            moveBody->setLinearVelocity(btVector3(speed.x,speed.y,speed.z));
            moveBody->setAngularVelocity(btVector3(0,0,0));
            moveBody->activate(true);
        }



        if(bento->getMouse(MOUSE_RIGHT)){
            if(!rightClicked){
                if(moveBody){
                    btVector3 impulse(direction.x, direction.y, direction.z);
                    moveBody->setLinearVelocity(btVector3(speed.x,speed.y,speed.z)+impulse*20);
                    moveBody->activate();
                    moveBody = nullptr;
                }else if (hitBody&& !hitBody->isStaticObject()) {
                    moveBody = hitBody;
                }
                rightClicked = true;
            }
        }else{
            rightClicked = false;
        }
        

        if(bento->getMouse(MOUSE_LEFT)){
            if(!leftClicked){
                if(moveBody){
                    moveBody = nullptr;
                }else if (hitBody&& !hitBody->isStaticObject()) {
                    glm::vec3 dir = glm::normalize(glm::vec3(rayTo.x()-rayFrom.x(),rayTo.y()-rayFrom.y(),rayTo.z()-rayFrom.z()))*25000.0f;
                    hitBody->applyImpulse(btVector3(dir.x, dir.y, dir.z),rayTo-hitBody->getWorldTransform().getOrigin());
                }
                leftClicked = true;
            }
        }else{
            leftClicked = false;
        }



        objects[1]->transformation = glm::translate(glm::mat4(1.0),hitPoint);

        if(bento->getKey(KEY_C)){
            for(int i = 0; i < 552; i++){
                float angle = i * 0.5f;
                float radius = 1.0f;
                float height = i * 1.5f;
            
                float x = cos(angle) * radius;
                float z = sin(angle) * radius;
                float y = height;
            
                btTransform transform;
                transform.setIdentity();
                transform.setOrigin(btVector3(x, y, z));
                btQuaternion rotation;
                rotation.setEuler(0.0, angle, 0.0);
                transform.setRotation(rotation);
                boxes[i]->setWorldTransform(transform);
            }
        }

        for(int i = 0; i < 552; i++){
            btTransform trans;
            boxes[i]->getMotionState()->getWorldTransform(trans);
            trans.getOpenGLMatrix(glm::value_ptr(objects[i+4]->transformation));
            boxes[i]->activate();
        }

        
        static glm::vec3 botPosition(-5,2,0), botSpeed(0,0,0);

        //botSpeed+=glm::normalize(glm::vec3(position.x - botPosition.x, 0.0f, position.z - botPosition.z))*glm::vec3(0.015);

        //botPosition += botSpeed;
        //botSpeed/=1.05;

        glm::vec3 toPlayer = glm::normalize(glm::vec3(position.x - botPosition.x, 0.0f, position.z - botPosition.z));
        float angle = atan2(toPlayer.x, toPlayer.z);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0, 1, 0));
        
        glm::mat4 translation = glm::scale(glm::translate(glm::mat4(1.0f), botPosition),glm::vec3(2));
        objects[2]->transformation = translation * rotation;

        float repelRadius = 3.0f;

        for (int i = 0; i < 552; ++i) {
            btVector3 pos = boxes[i]->getWorldTransform().getOrigin();
            glm::vec3 bodyPos = glm::vec3(pos.x(), pos.y(), pos.z());

            glm::vec3 dir = bodyPos - botPosition;
            float dist = glm::length(dir);
            if (dist < repelRadius && dist > 0.001f) {
                dir = glm::normalize(dir);
                boxes[i]->applyCentralImpulse(100000*btVector3(dir.x, dir.y, dir.z));
            }
        }
        if(fpsController){
            glm::vec3 dir = position - botPosition;
            float dist = glm::length(dir);
            if (dist < repelRadius+5.0f && dist > 0.001f) {
                dir = glm::normalize(dir);
                speed += glm::vec3(1000)*dir;
            }
        }
        
        if(fpsController)playerBody->setLinearVelocity(btVector3(speed.x,speed.y,speed.z));
        world->stepSimulation(deltaTime, 10);
        
        direction = glm::vec3(cos(angleY) * sin(angleX),sin(angleY),cos(angleY) * cos(angleX));
        right = glm::vec3(sin(angleX - 3.14f/2.0f),0,cos(angleX - 3.14f/2.0f));
        up = glm::cross(right,direction);
        float tilt = 0.003;
        if(pov==0){
            view = glm::lookAt(position-direction*tPDist,position-direction*(tPDist-1),up+glm::vec3(fclamp((speed.x+lastSpeed.x)*tilt,-1,1),0,fclamp((speed.z+lastSpeed.z)*tilt,-1,1)));
            bento->setUniform("view",view,true);
        }else{
            view = glm::lookAt(position,position+direction,up+glm::vec3(fclamp((speed.x+lastSpeed.x)*tilt,-1,1),0,fclamp((speed.z+lastSpeed.z)*tilt,-1,1)));
            bento->setUniform("view",view,true);
        }

        projection = glm::perspective(glm::radians(fov), fmax(viewSize.x,1.0f) / fmax(viewSize.y,1.0f), 0.01f, 100000.0f);
        bento->setUniform("projection",projection,true);

        bento->setActiveTextures(0,3);
        bento->setActiveDepthTexture(0);
        bento->setActiveAttachments(0,3);
        bento->normalizeTexture(2,false);
        bento->predrawTex(viewSize.x,viewSize.y);

        bento->setShader(defaultShader);
        bento->setUniform("tspecular",10.0f);
        for(Object* obj : objects)obj->drawTex(bento);

        bento->setVertices(groundMesh->getVertexBuffer());
        bento->setNormals(groundMesh->getNormalBuffer());
        bento->setUvs(groundMesh->getUVBuffer());
        bento->bindTexture(groundTex,0);
        model = glm::translate(glm::mat4(1.0),glm::vec3(0.0,0.1,0.0));
        bento->setUniform("model",model,true);
        bento->drawTex();

        model = glm::translate(glm::mat4(1.0),glm::vec3(0.0,1.5,0.0));
        bento->setUniform("view",glm::translate(view,glm::vec3(0.0,10.0,0.0)),true);
        bento->setUniform("model",model,true);
        for(int i = 0; i < vs.size(); i++)vs[i].x = vo[i]*1.25+sin(elapsedTime+vs[i].y*4)*0.125;

        bento->setUniform("view",view,true);

        bento->setVerticesDirect(vs);
        bento->setNormalsDirect(ns);
        bento->setUvsDirect(us);

        bento->drawTex();

        bento->bindTexture(handgunTex,0);
        bento->setVertices(handgun->getVertexBuffer());
        bento->setNormals(handgun->getNormalBuffer());
        bento->setUvs(handgun->getUVBuffer());




        float dAngleX = angleX-lastAngleX;
        float dAngleY = angleY-lastAngleY;
        static float lAngleX = 0.0f;
        static float lAngleY = 0.0f;
        lAngleX += (dAngleX-lAngleX)/15.0f;//you know in hindsight i probably should've made this a vec2
        lAngleY += (dAngleY-lAngleY)/15.0f;

        float dSpeedY = (leftClicked && !lastLeftClicked);

        static float lSpeedY = 0.0f;
        lSpeedY += (dSpeedY-lSpeedY)/15.0f;

        glm::mat4 gunMat = glm::translate(glm::mat4(1.0f), glm::vec3(viewSize.x/viewSize.y*1.1f, -1.0f, -2.0f+lSpeedY*10.0f));
        gunMat = glm::rotate(gunMat,lSpeedY*5.0f+lAngleY
            +(-2*((mousePos.y-windowPos.y-mouseSpacePos.y) / mouseSpaceSize.y)+1)*0.05f,glm::vec3(1,0,0));
        gunMat = glm::rotate(gunMat,-1.2f+lAngleX
            +(-2*((mousePos.x-windowPos.x-mouseSpacePos.x) / mouseSpaceSize.x)+1)*0.05f,glm::vec3(0,1,0));

        bento->setUniform("model", glm::inverse(view)*gunMat, true);
        bento->drawTex();

        bento->renderTex();

        bento->renderToTex(colTex,0);
        bento->renderToTex(nrmlTex,1);
        bento->renderToTex(posTex,2);

        bento->renderDepthToTex(depthTex,0);
        bento->setActiveTextures(3);
        bento->setActiveDepthTexture(1);
        bento->setActiveAttachments(0);
        bento->predrawTex(viewSize.x,viewSize.y);
        bento->setShader(screenShader);

        lights[0].position = position;
        updateLights(lights,bento,tPDist);//toilet paper distance
        bento->setUniform("numLights",(int)lights.size());
        static float tspecular = 10.0f;
        bento->setUniform("tspecular",tspecular);
        bento->setUniform("ambientColor",ambientColor);
        bento->bindTexture(colTex,0);
        bento->bindTexture(nrmlTex,1);
        bento->bindTexture(depthTex,2);
        bento->bindTexture(posTex,3);
        bento->setVerticesDirect(screenVertices);
        bento->setNormalsDirect(screenVertices);
        bento->setUvsDirect(screenUVs);
        bento->drawTex();
        bento->renderTex();
        bento->renderToTex(scrnTex,3);
        
        bento->setActiveTextures(4);
        bento->setActiveAttachments(0);

        //i can't be bothered to find out why shadows don't work on metal
        //either way it doesn't look right sometimes so i'll fix it later

        bento->predrawTex(viewSize.x,viewSize.y);
        bento->setShader(thresholdShader);
        static float threshold = 0.0;
        bento->setUniform("threshold",threshold);

        bento->bindTexture(scrnTex,0);
        bento->setVerticesDirect(screenVertices);
        bento->setNormalsDirect(screenVertices);
        bento->setUvsDirect(screenUVs);
        bento->drawTex();
        bento->renderTex();
        bento->renderToTex(blurTex,4);


        static int quality = 16;
        static int directions = 18;
        static float blurAmount = 15.0f;
        bento->setActiveTextures(5);
        bento->setActiveAttachments(0);
        bento->predrawTex(viewSize.x,viewSize.y);
        bento->setShader(boxBlurShader);

        bento->setUniform("blurAmount",glm::vec2(blurAmount)/glm::vec2(viewSize.x,viewSize.y));
        bento->setUniform("directions",directions);
        bento->setUniform("quality",quality);

        bento->bindTexture(blurTex,0);
        bento->setVerticesDirect(screenVertices);
        bento->setNormalsDirect(screenVertices);
        bento->setUvsDirect(screenUVs);
        bento->drawTex();
        bento->renderTex();
        bento->renderToTex(blurTex,5);

        static float exposure = 0.4f;
        static float strength = 1.2f;
        bento->setActiveTextures(6);
        bento->setActiveAttachments(0);
        bento->predrawTex(viewSize.x,viewSize.y);
        bento->setShader(combineShader);
        bento->setUniform("exposure",exposure);
        bento->setUniform("strength",strength);
        bento->bindTexture(scrnTex,0);
        bento->bindTexture(blurTex,1);
        bento->setVerticesDirect(screenVertices);
        bento->setNormalsDirect(screenVertices);
        bento->setUvsDirect(screenUVs);
        bento->drawTex();
        bento->renderTex();
        bento->renderToTex(scrnTex,6);


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


        ImGui::Begin("viewport");
        ImVec2 tsize = ImGui::GetWindowSize();
        ImVec2 tpos = ImGui::GetWindowPos();
        viewPos = glm::vec2(tpos.x,tpos.y);
        viewSize = glm::vec2(tsize.x,tsize.y-ImGui::GetFrameHeight());

        ImVec2 mouseSpaceMin = ImGui::GetWindowContentRegionMin();
        ImVec2 mouseSpacePosIM = ImVec2(viewPos.x+mouseSpaceMin.x,viewPos.y+mouseSpaceMin.y);
        ImVec2 mouseSpaceSizeIM = ImGui::GetContentRegionAvail();
        mouseSpacePos = glm::vec2(mouseSpacePosIM.x,mouseSpacePosIM.y);
        mouseSpaceSize = glm::vec2(mouseSpaceSizeIM.x,mouseSpaceSizeIM.y);       
        

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
        drawList->AddText(ImVec2(5+viewPos.x,30+viewPos.y),ImColor(255,255,255),(std::to_string(position.x)+","+std::to_string(position.y)+","+std::to_string(position.z)).c_str());
        drawList->AddText(ImVec2(5+viewPos.x,45+viewPos.y),ImColor(255,255,255),(std::to_string(speed.x)+","+std::to_string(speed.y)+","+std::to_string(speed.z)).c_str());
        drawList->AddText(ImVec2(5+viewPos.x,60+viewPos.y),ImColor(255,255,255),(std::to_string(glm::length(speed))).c_str());
        drawList->AddText(ImVec2(5+viewPos.x,75+viewPos.y),ImColor(255,255,255),(std::to_string(sqrt(speed.x*speed.x+speed.z*speed.z))).c_str());
        drawList->AddText(ImVec2(5+viewPos.x,90+viewPos.y),ImColor(255,255,255),defaultShader->getUni().c_str());
        centerX = tsize.x-min(0.075*tsize.x,0.075*tsize.y)+viewPos.x-2;
        centerY = tsize.y-min(0.075*tsize.x,0.075*tsize.y)+viewPos.y-2;
        float compassSize = min(0.075*tsize.x,0.075*tsize.y);
        drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX,centerY+compassSize*sin(angleY-pi/2)),ImColor(255,0,0),2);
        drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX+compassSize*cos(-angleX+pi),centerY+compassSize*sin(-angleX+pi)*cos(angleY-pi/2)),ImColor(0,255,0),2);
        drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX+compassSize*cos(-angleX+3*pi/2),centerY+compassSize*sin(-angleX+3*pi/2)*cos(angleY-pi/2)),ImColor(0,0,255),2);

        drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX,centerY+compassSize*sin(angleY+pi/2)),ImColor(200,0,0),2);
        drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX+compassSize*cos(-angleX),centerY+compassSize*sin(-angleX)*cos(angleY-pi/2)),ImColor(0,200,0),2);
        drawList->AddLine(ImVec2(centerX,centerY),ImVec2(centerX+compassSize*cos(-angleX+pi/2),centerY+compassSize*sin(-angleX+pi/2)*cos(angleY-pi/2)),ImColor(0,0,200),2);

        drawList->AddText(ImVec2(centerX+compassSize*cos(-angleX+pi),centerY+compassSize*sin(-angleX+pi)*cos(angleY-pi/2)),ImColor(0,255,0),"+X");
        drawList->AddText(ImVec2(centerX,centerY+compassSize*sin(angleY-pi/2)),ImColor(255,0,0),"+Y");
        drawList->AddText(ImVec2(centerX+compassSize*cos(-angleX+3*pi/2),centerY+compassSize*sin(-angleX+3*pi/2)*cos(angleY-pi/2)),ImColor(0,0,255),"+Z");

        drawList->AddText(ImVec2(centerX+compassSize*cos(-angleX),centerY+compassSize*sin(-angleX)*cos(angleY-pi/2)),ImColor(0,200,0),"-X");
        drawList->AddText(ImVec2(centerX,centerY+compassSize*sin(angleY+pi/2)),ImColor(200,0,0),"-Y");
        drawList->AddText(ImVec2(centerX+compassSize*cos(-angleX+pi/2),centerY+compassSize*sin(-angleX+pi/2)*cos(angleY-pi/2)),ImColor(0,0,200),"-Z");


        drawList->AddEllipse({centerX,centerY},ImVec2(10,10),compassSize*cos(angleY-pi/2),ImColor(150,150,150));
        drawList->AddCircle({centerX,centerY},compassSize,ImColor(200,200,200));
        viewportHover = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        viewportFocus = ImGui::IsWindowFocused();

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
        ImGui::Image((ImTextureID)posTex->getTexture(), imageSize);
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

            #ifdef MACOS
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
            #endif
        }

        ImGui::End();

        ImGui::PopStyleVar(2);

        ImGui::Begin("settings",nullptr,0);
        
        if (ImGui::CollapsingHeader("movement settings")) {

            ImGui::DragFloat("fov", &fov, 0.1f);

            ImGui::Checkbox("free cam", &freeCam);
            ImGui::Checkbox("ijkl as turn keys", &ijklAsLookKeys);
            ImGui::Checkbox("turn until escape", &turnUntilEscape);
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
                if(pov==0)fpsController = false;
            }
            if(pov==0)ImGui::BeginDisabled();
            if(ImGui::Checkbox("first person controller",&fpsController)){
                freeCam = (fpsController==false);
                if(fpsController)turnUntilEscape = true;
                wrapMouse=false;
                lockMouse=true;
            }
            if(pov==0)ImGui::EndDisabled();
            
            ImGui::Indent(20.0f);
                
                ImGui::Checkbox("wasd to move",&wasdToMove);

                if(!wasdToMove)ImGui::BeginDisabled();
                ImGui::Checkbox("fast shift",&shiftToGoFast);
                if(!shiftToGoFast)ImGui::BeginDisabled();
                ImGui::DragFloat("fast speed",&fastSpeed,0.001f,0.001f,FLT_MAX);
                ImGui::DragFloat("slow speed",&slowSpeed,0.001f,0.001f,FLT_MAX);
                if(!wasdToMove)ImGui::EndDisabled();
                if(!shiftToGoFast)ImGui::EndDisabled();

                ImGui::Checkbox("invert X",&invTurnX);
                ImGui::DragFloat("X sensitivity",&turnSensX,0.001f,0.001f,FLT_MAX);
                ImGui::Checkbox("invert Y",&invTurnY);
                ImGui::DragFloat("Y sensitivity",&turnSensY,0.001f,0.001f,FLT_MAX);

                ImGui::Checkbox("shift to move", &shiftToMove);

                if(!shiftToMove)ImGui::BeginDisabled();
                    ImGui::Checkbox("invert move X",&invShiftMoveX);
                    ImGui::DragFloat("X move sensitivity",&shiftMoveSensX,0.001f,0.001f,FLT_MAX);
                    ImGui::Checkbox("invert move Y",&invShiftMoveY);
                    ImGui::DragFloat("Y move sensitivity",&shiftMoveSensY,0.001f,0.001f,FLT_MAX);
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
            static float clearColor[4] = {142.0/255.0,169.0/255.0,168.0/255.0,1};
            static float ambColor[4] = {142.0/255.0,169.0/255.0,168.0/255.0,1};
            ImGui::ColorPicker4("sky color", (float*)&clearColor);
            bento->setClearColor(glm::vec4(clearColor[0],clearColor[1],clearColor[2],clearColor[3]));
            static bool amCisSkC = true;
            ImGui::Checkbox("ambient color = sky color?", &amCisSkC);
            if(amCisSkC){
                ambientColor.x = clearColor[0];
                ambientColor.y = clearColor[1];
                ambientColor.z = clearColor[2];
            }else{
                ImGui::ColorPicker3("ambient color", (float*)&ambColor);
                ambientColor.x = ambColor[0];
                ambientColor.y = ambColor[1];
                ambientColor.z = ambColor[2];
            }
        }
        static bool showDemo = false;
        
        ImGui::Checkbox("show demo", &showDemo);
        
        if (ImGui::CollapsingHeader("shader settings")) {
            ImGui::DragFloat("specular amount", &tspecular, 0.1f);

            ImGui::DragFloat("threshold", &threshold, 0.1f);

            ImGui::DragInt("quality", &quality);
            ImGui::DragInt("directions", &directions);
            ImGui::DragFloat("blur amount", &blurAmount, 0.1f);

            ImGui::DragFloat("exposure", &exposure, 0.1f);
            ImGui::DragFloat("strength", &strength, 0.1f);
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
            ImGui::DragFloat3("position", glm::value_ptr(light.position));
            ImGui::ColorEdit3("ambient", glm::value_ptr(light.ambient));
            ImGui::ColorEdit3("diffuse", glm::value_ptr(light.diffuse));
            ImGui::ColorEdit3("specular", glm::value_ptr(light.specular));
            ImGui::DragFloat("constant",&light.constant,0.01f);
            ImGui::DragFloat("linear",&light.linear,0.01f);
            ImGui::DragFloat("quadratic",&light.quadratic,0.001f);
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

void updateLights(std::vector<Light> lights,Bento* bento,float tPDist){
    for (int i = 0; i < (int)lights.size(); ++i) {
        positions[i] = lights[i].position;
        constants[i] = lights[i].constant;
        linears[i] = lights[i].linear;
        quadratics[i] = lights[i].quadratic;
        ambients[i] = lights[i].ambient;
        diffuses[i] = lights[i].diffuse;
        speculars[i] = lights[i].specular;
    }
    bento->setUniform("positions",positions,50);
    bento->setUniform("constants",constants,50);
    bento->setUniform("linears",linears,50);
    bento->setUniform("quadratics",quadratics,50);
    bento->setUniform("ambients",ambients,50);
    bento->setUniform("diffuses",diffuses,50);
    bento->setUniform("speculars",speculars,50);
    bento->setUniform("pos",position-direction*tPDist);
}