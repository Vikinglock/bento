#ifndef BENTO_H
#define BENTO_H
#ifdef USE_METAL
#include "metal/metal.h"
class Bento : public MetalBento {};
#elif USE_OPENGL
#include "opengl/opengl.h"
class Bento : public OpenGLBento {};
#else
#include "opengl/opengl.h"
class Bento : public OpenGLBento {};
#endif

#define pi 3.1415926535

//also very sorry if this is a really unorthodox method of drawing stuff or handling graphics

void loadOBJ(const char *path, std::vector<glm::vec3> &out_vertices, std::vector<glm::vec2> &out_uvs, std::vector<glm::vec3> &out_normals);

//I LOVE ENUMS I LOVE ENUMS I LOVE ENUMS
enum{
    PhysicsObject,
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
    Object(const char *n, glm::vec3 pos, Mesh m, Texture* tex):mesh(m),texture(tex){
        name = n;
        position = pos;
    }
    Object(const char *n, glm::vec3 pos, Mesh m):mesh(m),texture(nullptr){
        name = n;
        position = pos;
    }
    Object(const char *n, glm::vec3 pos, Texture* tex):mesh(nullptr),texture(tex){
        name = n;
        position = pos;
    }
    Object(const char *n, glm::vec3 pos, const char *meshPath, const char *texPath):mesh(Mesh(meshPath)),texture(new Texture(texPath)){
        name = n;
        position = pos;
    }
    Object(const char *n, glm::vec3 pos, const char *meshPath):mesh(Mesh(meshPath)),texture(nullptr){
        name = n;
        position = pos;
    }
    ~Object(){

    }
    void draw(Bento *bento){
        bento->setVertices(mesh.getVertexBuffer());
        bento->setNormals(mesh.getNormalBuffer());
        bento->setUvs(mesh.getUVBuffer());
        bento->setModelMatrix(glm::translate(glm::mat4(1.0),position));
        if(texture){
            bento->bindTexture(texture);
        }

        bento->draw();
    }
    const Mesh& getMesh() const { return mesh; }
    const Texture* getTexture() const { return texture; }
private:
    std::string name;
    glm::vec3 position;
    Mesh mesh;
    Texture* texture;
};


void loadOBJ(const char *path, std::vector<glm::vec3> &out_vertices, std::vector<glm::vec2> &out_uvs, std::vector<glm::vec3> &out_normals) {
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("Error opening file %s\n", path);
        return;
    }

    char lineHeader[1000];
    while (fscanf(file, "%s", lineHeader) != EOF) {
        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
        } else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            uv.y = uv.y;
            temp_uvs.push_back(uv);
        } else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        } else if (strcmp(lineHeader, "f") == 0) {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%u/%u/%u %u/%u/%u %u/%u/%u\n",
                                 &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                                 &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                                 &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if (matches != 9) {
                printf("Error reading face data\n");
                fclose(file);
                return;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        } else {
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
    }

    for (size_t i = 0; i < vertexIndices.size(); i++) {
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        glm::vec2 uv = temp_uvs[uvIndex - 1];
        glm::vec3 normal = temp_normals[normalIndex - 1];

        out_vertices.push_back(vertex);
        out_uvs.push_back(uv);
        out_normals.push_back(normal);
    }
    fclose(file);
}
#endif