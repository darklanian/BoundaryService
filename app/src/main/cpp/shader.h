//
// Created by GRAM on 2022-05-09.
//

#ifndef BOUNDARYSERVICE_SHADER_H
#define BOUNDARYSERVICE_SHADER_H

GLuint create_shader(AAssetManager* am, GLenum shader_type, const char* filename);
GLuint create_program(AAssetManager* am, const char *vertex_shader_filename, const char *fragment_shader_filename, const char *geometry_shader_filename = nullptr);

#endif //BOUNDARYSERVICE_SHADER_H
