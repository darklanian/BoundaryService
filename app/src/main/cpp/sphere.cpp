//
// Created by GRAM on 2022-05-06.
//
#include <GLES3/gl32.h>
#include <glm/glm.hpp>
#include <vector>
#include <sstream>
#include <android_native_app_glue.h>
#include "common/gfxwrapper_opengl.h"
#include <common/xr_linear.h>
#include "logger.h"

std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;

void load_object(AAssetManager* am, const char* filename) {
    AAsset* asset = AAssetManager_open(am, filename, AASSET_MODE_BUFFER);
    const char* buffer = (const char*)AAsset_getBuffer(asset);
    std::stringstream ss(buffer);
    std::string line;
    while (std::getline(ss, line)) {
        if (strncmp(line.c_str(), "vt", 2) == 0) {
            glm::vec2 uv;
            sscanf(line.c_str(), "vt %f %f", &uv.x, &uv.y);
            uvs.push_back(uv);
        } else if (strncmp(line.c_str(), "vn", 2) == 0) {
            glm::vec3 normal;
            sscanf(line.c_str(), "vn %f %f %f", &normal.x, &normal.y, &normal.z);
            normals.push_back(normal);
        } else if (strncmp(line.c_str(), "v", 1) == 0) {
            glm::vec3 vertex;
            sscanf(line.c_str(), "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            vertices.push_back(vertex);
        } else if (strncmp(line.c_str(), "f", 1) == 0) {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            vertexIndices.push_back(vertexIndex[0]-1);
            vertexIndices.push_back(vertexIndex[1]-1);
            vertexIndices.push_back(vertexIndex[2]-1);
            uvIndices    .push_back(uvIndex[0]-1);
            uvIndices    .push_back(uvIndex[1]-1);
            uvIndices    .push_back(uvIndex[2]-1);
            normalIndices.push_back(normalIndex[0]-1);
            normalIndices.push_back(normalIndex[1]-1);
            normalIndices.push_back(normalIndex[2]-1);
        }
    }
    AAsset_close(asset);
}

GLuint sphere_vao;
GLuint sphere_vbo;
GLuint sphere_ibo;

void sphere_init(AAssetManager* am) {
    load_object(am, "sphere.obj");

    glGenVertexArrays(1, &sphere_vao);
    glGenBuffers(1, &sphere_vbo);
    glGenBuffers(1, &sphere_ibo);

    glBindVertexArray(sphere_vao);

    glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size()*sizeof(unsigned int), &vertexIndices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
}

void sphere_deinit() {
    glDeleteVertexArrays(1, &sphere_vao);
    glDeleteBuffers(1, &sphere_vbo);
    glDeleteBuffers(1, &sphere_ibo);
}

void sphere_enable() {
    glBindVertexArray(sphere_vao);
}

void sphere_draw() {
    glDrawElements(GL_TRIANGLES, vertexIndices.size(), GL_UNSIGNED_INT, 0);
}

