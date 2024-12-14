#include "lib/glm/glm.hpp"
#include "lib/glm/gtc/matrix_transform.hpp"
#include "lib/glm/gtc/type_ptr.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <any>
#include <fstream>
#include <sstream>

#include "opengl.h"

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

int main() {
    std::vector<glm::vec3> vertices, normals;
    std::vector<glm::vec2> uvs;
    loadOBJ("../resources/suzanne.obj", vertices, uvs, normals);

    vertices.push_back(glm::vec3(1.0,0.0,0.0));
    vertices.push_back(glm::vec3(0.0,1.0,0.0));
    vertices.push_back(glm::vec3(-1.0,0.0,0.0));

    glm::vec2 windowSize(1000, 1000);
    glm::vec3 position(0.0f, 1.0f, 3.0f), direction(0.0f, 0.0f, -1.0f), up(0, 1, 0);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(position, position + direction, up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), windowSize.x / windowSize.y, 0.1f, 100.0f);
    glm::mat4 mvp = projection * view * model;
    

    OpenGLBento *renderer = new OpenGLBento();
    
    renderer->init("ベント",1000,1000);

    while(renderer->isRunning()) {
        renderer->setVertices(vertices);
        renderer->setNormals(normals);
        renderer->setUvs(uvs);
        renderer->setMVPMatrix(mvp);


        renderer->render();
    }

    glfwTerminate();

    return 0;
}