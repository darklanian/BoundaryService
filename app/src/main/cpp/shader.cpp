//
// Created by GRAM on 2022-05-09.
//
#include <android_native_app_glue.h>
#include <GLES3/gl32.h>
#include <cstdlib>

#include "logger.h"

void checkGlError(const char* name) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        LOGE("%s error: 0x%08X", name, error);
}

GLuint create_shader(AAssetManager* am, GLenum shader_type, const char* filename) {
    AAsset* asset = AAssetManager_open(am, filename, AASSET_MODE_BUFFER);
    int len = AAsset_getLength(asset);
    const char* src = (const char*)AAsset_getBuffer(asset);
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &src, &len);
    GLint compiled = GL_FALSE;
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    AAsset_close(asset);
    if (!compiled) {
        GLint infoLogLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            glGetShaderInfoLog(shader, infoLogLen, NULL, infoLog);
            LOGE("Could not compile %s shader:\n%s", shader_type == GL_VERTEX_SHADER?"vertext":"fragment", infoLog);
            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint create_program(AAssetManager* am, const char *vertex_shader_filename, const char *fragment_shader_filename, const char *geometry_shader_filename) {
    GLuint shader_vertex = create_shader(am, GL_VERTEX_SHADER, vertex_shader_filename);
    GLuint shader_fragment = create_shader(am, GL_FRAGMENT_SHADER, fragment_shader_filename);
    GLuint shader_geometry = 0;
    LOGI("geometry_shader_filename: %s", geometry_shader_filename);
    if (geometry_shader_filename != nullptr)
        shader_geometry = create_shader(am, GL_GEOMETRY_SHADER, fragment_shader_filename);

    LOGI("shader_geometry: %d", shader_geometry);
    GLuint program = glCreateProgram();
    if (!program) {
        LOGE("Could not create a program: 0x%08X", glGetError());
        glDeleteShader(shader_vertex);
        glDeleteShader(shader_fragment);
        if (shader_geometry)
            glDeleteShader(shader_geometry);
        return 0;
    }
    glAttachShader(program, shader_vertex);
    glAttachShader(program, shader_fragment);
    if (shader_geometry)
        glAttachShader(program, shader_geometry);
    glLinkProgram(program);
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        LOGE("Could not link program");
        GLint infoLogLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);
        if (infoLogLen > 0) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLen);
            glGetProgramInfoLog(program, infoLogLen, NULL, infoLog);
            LOGE("%s", infoLog);
            free(infoLog);
        }
        glDeleteProgram(program);
        return 0;
    }
    glDeleteShader(shader_fragment);
    glDeleteShader(shader_vertex);
    if (shader_geometry)
        glDeleteShader(shader_geometry);
    return program;
}
