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
#include "sphere.h"

float boundary_bottom = -10.0f;
float boundary_height = 30.0f;
float boundary_scale = 10.0f;

std::vector<float> boundary_points;
std::vector<float> boundary_uv;

GLuint boundary_vao;
GLuint boundary_vbo;
GLuint boundary_vbo_uv;

GLuint program_grid;

GLuint program_surface;
GLint color_location;

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
	program_surface = create_program(am, "surface.vertex.shader", "surface.fragment.shader");
	color_location = glGetUniformLocation(program_surface, "vColor");

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

	sphere_init(am);
}

void boundary_deinit() {
	glDeleteBuffers(1, &boundary_vbo);
	glDeleteBuffers(1, &boundary_vbo_uv);
	glDeleteVertexArrays(1, &boundary_vao);
	glDeleteProgram(program_grid);
	glDeleteProgram(program_surface);

	sphere_deinit();
}

void boundary_draw(GLuint program, XrMatrix4x4f vp) {
	glUseProgram(program);

	glUniformMatrix4fv(2, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&vp));

	glBindVertexArray(boundary_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, boundary_points.size() / 3);
	glBindVertexArray(0);
}

void boundary_draw_grid(XrMatrix4x4f vp) {
	glUseProgram(program_grid);

	glUniformMatrix4fv(2, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&vp));

	glBindVertexArray(boundary_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, boundary_points.size() / 3);
	glBindVertexArray(0);
}

float c_white[] = {1.0f, 1.0f, 1.0f, 1.0f};
float c_red[] = {1.0f, 0.0f, 0.0f, 1.0f};
float c_green[] = {0.0f, 1.0f, 0.0f, 1.0f};
float c_blue[] = {0.0f, 0.0f, 1.0f, 1.0f};
float c_transparent[] = {0.0f, 0.0f, 0.0f, 0.0f};

float debug_z = -10.0f;
void set_debug_z(float z) { debug_z = z; }

void draw_sphere_on_surface(XrMatrix4x4f vp, float s) {
	XrVector3f translation {0.0f, 0.0f, debug_z};
	XrQuaternionf rotation {0.0f, 0.0f, 0.0f, 1.0f};
	XrVector3f scale {s, s, s};
	XrMatrix4x4f model;
	XrMatrix4x4f_CreateTranslationRotationScale(&model, &translation, &rotation, &scale);
	XrMatrix4x4f mvp;
	XrMatrix4x4f_Multiply(&mvp, &vp, &model);
	glUniformMatrix4fv(2, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&mvp));

	sphere_enable();

	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_GREATER);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	sphere_draw();

	glDepthFunc(GL_LESS);
	glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	sphere_draw();

	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);
}

void boundary_draw_surface(XrMatrix4x4f vp) {
	glUseProgram(program_surface);

	glUniformMatrix4fv(2, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&vp));
	glUniform4fv(color_location, 1, c_white);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glBindVertexArray(boundary_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, boundary_points.size() / 3);
	glBindVertexArray(0);
	// boundary wall done

	glUniform4fv(color_location, 1, c_red);
	draw_sphere_on_surface(vp, 3.0f);
	//glDisable(GL_BLEND);
	glUniform4fv(color_location, 1, c_transparent);
	draw_sphere_on_surface(vp, 2.5f);
	//glEnable(GL_BLEND);
}
