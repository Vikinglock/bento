#ifndef BENTO_H
#define BENTO_H
#ifdef USE_METAL
#include "metal/metal.h"
#elif USE_OPENGL
#include "opengl/opengl.h"
#else
#include "opengl/opengl.h"
#endif

#define pi 3.1415926535

//also very sorry if this is a really unorthodox method of drawing stuff or handling graphics        but i don't care

#include "sound/sound.h"

void loadOBJ(const char *path, std::vector<glm::vec3> &out_vertices, std::vector<glm::vec2> &out_uvs, std::vector<glm::vec3> &out_normals);

//I LOVE ENUMS I LOVE ENUMS I LOVE ENUMS
enum{
    PhysicsObject,//what was this supposed to be again?
    NoPhysicsObject,
};

class Mesh{
public:
    Mesh(const char *path){
        loadOBJ(path,vertices,uvs,normals);
        vBuffer.setBuffer(vertices);
        nBuffer.setBuffer(normals);
        uBuffer.setBuffer(uvs);
    }
    ~Mesh(){
        delete&vertices;
        delete&normals;
        delete&uvs;
        delete&vBuffer;
        delete&nBuffer;
        delete&uBuffer;
    }
    std::vector<glm::vec3> getVertices()const{return vertices;}
    std::vector<glm::vec3> getNormals()const{return normals;}
    std::vector<glm::vec2> getUVs()const{return uvs;}
    vertexBuffer getVertexBuffer()const{return vBuffer;}
    normalBuffer getNormalBuffer()const{return nBuffer;}
    uvBuffer getUVBuffer()const{return uBuffer;}

private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    vertexBuffer vBuffer;
    normalBuffer nBuffer;
    uvBuffer uBuffer;
};

class Object{
public:
    Object(const char *n, glm::mat4 t, Mesh* m, Texture* tex):mesh(m),texture(tex){
        name = n;
        transformation = t;
    }
    Object(const char *n, glm::mat4 t, Mesh* m):mesh(m),texture(nullptr){
        name = n;
        transformation = t;
    }
    Object(const char *n, glm::mat4 t, Texture* tex):mesh(nullptr),texture(tex){
        name = n;
        transformation = t;
    }
    Object(const char *n, glm::mat4 t, const char *meshPath, const char *texPath):mesh(new Mesh(meshPath)),texture(new Texture(texPath)){
        name = n;
        transformation = t;
    }
    Object(const char *n, glm::mat4 t, const char *meshPath):mesh(new Mesh(meshPath)),texture(nullptr){
        name = n;
        transformation = t;
    }
    ~Object(){

    }
    void draw(Bento *bento){
        bento->setVertices(mesh->getVertexBuffer());
        bento->setNormals(mesh->getNormalBuffer());
        bento->setUvs(mesh->getUVBuffer());
        bento->setUniform("model",transformation,true);
        if(texture){
            bento->bindTexture(texture,0);
        }

        bento->draw();
    }
    void drawTex(Bento *bento){
        bento->setVertices(mesh->getVertexBuffer());
        bento->setNormals(mesh->getNormalBuffer());
        bento->setUvs(mesh->getUVBuffer());
        bento->setUniform("model",transformation,true);
        if(texture){
            bento->bindTexture(texture,0);
        }

        bento->drawTex();
    }
    const Mesh* getMesh() const { return mesh; }
    const Texture* getTexture() const { return texture; }

    glm::mat4 transformation;
private:
    std::string name;
    Mesh* mesh;
    Texture* texture;
};


void loadOBJ(const char *path, std::vector<glm::vec3> &out_vertices, std::vector<glm::vec2> &out_uvs, std::vector<glm::vec3> &out_normals) {
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    
    std::ifstream file(path);
    if (!file.is_open())printf("Error opening file %s\n", path);
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        if (type == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        }
        else if (type == "vt") {
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (type == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (type == "f") {
            std::vector<int> vertexIndices;
            std::vector<int> uvIndices;
            std::vector<int> normalIndices;
            
            std::string faceToken;
            while (iss >> faceToken) {
                int vertexIndex = -1;
                int uvIndex = -1;
                int normalIndex = -1;
                
                std::istringstream tokenStream(faceToken);
                std::string segment;
                std::getline(tokenStream, segment, '/');
                if(!segment.empty())vertexIndex = std::stoi(segment) - 1;
                std::getline(tokenStream, segment, '/');
                if(!segment.empty())uvIndex = std::stoi(segment) - 1;
                std::getline(tokenStream, segment);
                if(!segment.empty())normalIndex = std::stoi(segment) - 1;
                vertexIndices.push_back(vertexIndex);
                uvIndices.push_back(uvIndex);
                normalIndices.push_back(normalIndex);
            }
            for (size_t i = 1; i < vertexIndices.size() - 1; i++) {
                if(vertexIndices[0] >= 0 && vertexIndices[0] < temp_vertices.size())out_vertices.push_back(temp_vertices[vertexIndices[0]]);
                if(uvIndices[0] >= 0 && uvIndices[0] < temp_uvs.size())out_uvs.push_back(temp_uvs[uvIndices[0]]);
                else if(!temp_uvs.empty())out_uvs.push_back(glm::vec2(0.0f, 0.0f));
                if(normalIndices[0] >= 0 && normalIndices[0] < temp_normals.size())out_normals.push_back(temp_normals[normalIndices[0]]);
                else if (!temp_normals.empty())out_normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));

                if(vertexIndices[i] >= 0 && vertexIndices[i] < temp_vertices.size())out_vertices.push_back(temp_vertices[vertexIndices[i]]);
                if(uvIndices[i] >= 0 && uvIndices[i] < temp_uvs.size())out_uvs.push_back(temp_uvs[uvIndices[i]]);
                else if(!temp_uvs.empty())out_uvs.push_back(glm::vec2(0.0f, 0.0f));
                if(normalIndices[i] >= 0 && normalIndices[i] < temp_normals.size())out_normals.push_back(temp_normals[normalIndices[i]]);
                else if(!temp_normals.empty())out_normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
                
                if(vertexIndices[i+1] >= 0 && vertexIndices[i+1] < temp_vertices.size())out_vertices.push_back(temp_vertices[vertexIndices[i+1]]);
                if(uvIndices[i+1] >= 0 && uvIndices[i+1] < temp_uvs.size())out_uvs.push_back(temp_uvs[uvIndices[i+1]]);
                else if(!temp_uvs.empty())out_uvs.push_back(glm::vec2(0.0f, 0.0f));
                if(normalIndices[i+1] >= 0 && normalIndices[i+1] < temp_normals.size())out_normals.push_back(temp_normals[normalIndices[i+1]]);
                else if(!temp_normals.empty())out_normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
            }
        }
    }
}
#endif