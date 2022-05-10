//
// Created by kwangil.ji on 2022-05-10.
//
#include <vector>
#include <cmath>

#include <GLES3/gl32.h>

#include <android_native_app_glue.h>

#include "common/gfxwrapper_opengl.h"
#include <common/xr_linear.h>

#include "logger.h"
#include "shader.h"

float boundary_bottom = -10.0f;
float boundary_height = 30.0f;
float boundary_scale = 50.0f;

std::vector<float> boundary_points;
std::vector<float> boundary_uv;

GLuint boundary_vao;
GLuint boundary_vbo;
GLuint boundary_vbo_uv;

GLuint program_grid;
GLuint matrix_location_grid;

GLuint program_surface;
GLuint matrix_location_surface;

void add_point(float x, float y, float z) {
	boundary_points.push_back(x);
	boundary_points.push_back(y);
	boundary_points.push_back(z);
}

void add_point_uv(float u, float v) {
	boundary_uv.push_back(u);
	boundary_uv.push_back(v);
}

float get_distance(float x1, float y1, float x2, float y2) {
	float dx = x2-x1;
	float dy = y2-y1;
	return std::sqrt(dx*dx + dy*dy);
}

void load_boundary_geometry() {
	FILE* fp = fopen("/sdcard/points.txt", "rt");
	if (fp == nullptr)
		return;

	float x, z;
	char* line = nullptr;
	size_t len;

	float px = 0.0f, pz = 0.0f;
	float d = 0.0f;
	while (getline(&line, &len, fp) != -1) {
		if (2 == sscanf(line, "%f %f", &x, &z)) {
			x *= boundary_scale;
			z *= boundary_scale;
			add_point(x, boundary_bottom, z);
			add_point(x, boundary_bottom+boundary_height, z);
			if (px != 0.0f && pz != 0.0f)
				d += get_distance(px, pz, x, z);
			add_point_uv(d, 0.0f);
			add_point_uv(d, boundary_height);
			px = x;
			pz = z;
		}
	}
	add_point(boundary_points[0], boundary_points[1], boundary_points[2]);
	add_point(boundary_points[3], boundary_points[4], boundary_points[5]);
	d += get_distance(px, pz, boundary_points[0], boundary_points[2]);
	add_point_uv(d, 0.0f);
	add_point_uv(d, boundary_height);

	free(line);
	fclose(fp);

	LOGI("boundary vertex count: %d", (int)boundary_points.size()/3);
}

void boundary_init(AAssetManager* am) {
	program_grid = create_program(am, "grid.vertex.shader", "grid.fragment.shader");
	matrix_location_grid = glGetUniformLocation(program_grid, "MVP");

	program_surface = create_program(am, "surface.vertex.shader", "surface.fragment.shader");
	matrix_location_surface = glGetUniformLocation(program_surface, "MVP");

	load_boundary_geometry();

	glGenVertexArrays(1, &boundary_vao);
	glGenBuffers(1, &boundary_vbo);
	glGenBuffers(1, &boundary_vbo_uv);

	glBindVertexArray(boundary_vao);
	glBindBuffer(GL_ARRAY_BUFFER, boundary_vbo);
	glBufferData(GL_ARRAY_BUFFER, boundary_points.size() * 4 /*bytes*/ * 3, boundary_points.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, boundary_vbo_uv);
	glBufferData(GL_ARRAY_BUFFER, boundary_uv.size() * 4 * 2, boundary_uv.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void boundary_deinit() {
	glDeleteBuffers(1, &boundary_vbo);
	glDeleteBuffers(1, &boundary_vbo_uv);
	glDeleteVertexArrays(1, &boundary_vao);
	glDeleteProgram(program_grid);
	glDeleteProgram(program_surface);
}

void boundary_draw(XrMatrix4x4f vp, GLuint program) {
	glUseProgram(program);

	XrVector3f translation {0.0f, 0.0f, 0.0f};
	XrQuaternionf rotation {0.0f, 0.0f, 0.0f, 1.0f};
	XrVector3f scale {1.0f, 1.0f, 1.0f};
	XrMatrix4x4f model;
	XrMatrix4x4f_CreateTranslationRotationScale(&model, &translation, &rotation, &scale);
	XrMatrix4x4f mvp;
	XrMatrix4x4f_Multiply(&mvp, &vp, &model);
	glUniformMatrix4fv(matrix_location_grid, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&mvp));

	glBindVertexArray(boundary_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, boundary_points.size() / 3);

	glBindVertexArray(0);
}

void boundary_draw_grid(XrMatrix4x4f vp) {
	boundary_draw(vp, program_grid);
}


void boundary_draw_surface(XrMatrix4x4f vp) {
	boundary_draw(vp, program_surface);
}
