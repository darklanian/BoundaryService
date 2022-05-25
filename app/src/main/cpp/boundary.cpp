//
// Created by kwangil.ji on 2022-05-10.
//
#include <vector>
#include <cmath>
#include <limits>

#include <GLES3/gl32.h>

#include <android_native_app_glue.h>

#include "common/gfxwrapper_opengl.h"
#include <common/xr_linear.h>

#include "logger.h"
#include "shader.h"
#include "sphere.h"

float boundary_bottom = -10.0f;
float boundary_height = 30.0f;
float boundary_scale = 20.0f;

std::vector<float> boundary_points;
std::vector<float> boundary_uv;

GLuint boundary_vao;
GLuint boundary_vbo;
GLuint boundary_vbo_uv;

GLuint program_grid;

GLuint program_surface;
GLint color_location;

float hand_z = -17.0f;  // for debug
void set_debug_z(float z) {
	hand_z = z;
	LOGI("hand_z = %f", hand_z);
}

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

float get_distance_to_line(float x1, float y1, float x2, float y2, float x, float y, /*nearest point*/float* nx, float* ny) {
	float theta = std::atanf((y2-y1)/(x2-x1)) - std::atan((y-y1)/(x-x1));
	if (theta >= M_PI/2) {
		if (nx)
			*nx = x1;
		if (ny)
			*ny = y1;
		return get_distance(x1, y1, x, y);
	} else if (theta <= M_PI/4) {
		if (nx)
			*nx = x2;
		if (ny)
			*ny = y2;
		return get_distance(x2, y2, x, y);
	} else {
		float a = (y2-y1);
		float b = -(x2-x1);
		float c = -(y2-y1)*x1 + (x2-x1)*y1;
		float k = -(a*x+b*y+c)/(a*a+b*b);
		if (nx)
			*nx = x+a*k;
		if (ny)
			*ny = y+b*k;
		return abs(a*x+b*y+c)/sqrt(a*a+b*b);
	}
}

float get_distance_to_boundary(float x, float y, /*nearest point*/float* nx, float* ny) {
	float min = std::numeric_limits<float>::max();
	float x1 = boundary_points[0]; // x-z plane: x
	float y1 = boundary_points[2]; // x-z plane: z
	for (int i = 6; i < boundary_points.size()/6; ++i) {
		float x2 = boundary_points[i*6]; // x-z plane: x
		float y2 = boundary_points[i*6+2]; // x-z plane: z
		float tx = 0.0f, ty = 0.0f;
		float d = get_distance_to_line(x1, y1, x2, y2, x, y, &tx, &ty);
		if (d < min) {
			min = d;
			if (nx)
				*nx = tx;
			if (ny)
				*ny = ty;
		}
		x1 = x2;
		y1 = y2;
	}

	return min;
}

XrVector3f head_position = {0.0f, 0.0f, 0.0f};
XrVector3f hand_position[2] = {
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f},
};
XrVector3f nearest_point = {0.0f, 0.0f, 0.0f};
float distance_to_boundary = 0.0f;

void boundary_set_head_position(XrVector3f hp) {
	head_position = hp;
	distance_to_boundary = get_distance_to_boundary(hp.x, hp.z, &nearest_point.x, &nearest_point.z);
	LOGI("DEBUG: head: %f,%f,%f", hp.x, hp.y, hp.z);
	//LOGI("DEBUG: distance_to_boundary: %f", distance_to_boundary);
	nearest_point.y = boundary_bottom;
}

void boundary_set_hand_position(XrVector3f hp, int hand) {
	hand_position[hand] = hp;
	hand_position[hand].z += hand_z;
	float d, nx, ny;
	d = get_distance_to_boundary(hp.x, hp.z, &nx, &ny);
	LOGI("DEBUG: hand[%d]: %f,%f,%f", hand, hp.x, hp.y, hp.z);
	if (d < distance_to_boundary) {
		distance_to_boundary = d;
		nearest_point.x = nx;
		nearest_point.y = boundary_bottom;
		nearest_point.z = ny;
	}
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

void boundary_draw_grid(XrMatrix4x4f vp) {
	glEnable(GL_BLEND);

	glUseProgram(program_grid);

	glUniformMatrix4fv(2, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&vp));
	glUniform1f(3, distance_to_boundary);

	glBindVertexArray(boundary_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, boundary_points.size() / 3);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
}

float c_white[] = {1.0f, 1.0f, 1.0f, 1.0f};
float c_red[] = {1.0f, 0.0f, 0.0f, 1.0f};
float c_green[] = {0.0f, 1.0f, 0.0f, 1.0f};
float c_blue[] = {0.0f, 0.0f, 1.0f, 1.0f};
float c_transparent[] = {0.0f, 0.0f, 0.0f, 0.0f};

void draw_sphere_on_surface(XrMatrix4x4f vp, float s, XrVector3f position) {
	XrQuaternionf rotation {0.0f, 0.0f, 0.0f, 1.0f};
	XrVector3f scale {s, s, s};
	XrMatrix4x4f model;
	XrMatrix4x4f_CreateTranslationRotationScale(&model, &position, &rotation, &scale);
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
	glClear(GL_STENCIL_BUFFER_BIT);
}

void boundary_draw_hole(XrMatrix4x4f vp, float size, float *color, XrVector3f position) {

	glUniform4fv(color_location, 1, color);
	draw_sphere_on_surface(vp, size, position);

	glUniform4fv(color_location, 1, c_transparent);
	draw_sphere_on_surface(vp, size*0.9f, position);
}

void boundary_draw_surface(XrMatrix4x4f vp) {
	glUseProgram(program_surface);

	glUniformMatrix4fv(2, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&vp));
	glUniform4fv(color_location, 1, c_white);

	// ---------------------------------------------------------------------------------------------
	//LOGI("distance to boundary: %f, nearest point: %.2f,%.2f", d, nx, ny);
	/*float line_vertices[] = {
		head_position.x, boundary_bottom, head_position.z,
		nearest_point.x, boundary_bottom, nearest_point.z,
	};
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);

	glLineWidth(10.0f);
	glDrawArrays(GL_LINES, 0, 2);

	glBindBuffer(GL_VERTEX_ARRAY, 0);
	glDeleteBuffers(1, &vbo);*/
	// ---------------------------------------------------------------------------------------------

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glBindVertexArray(boundary_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, boundary_points.size() / 3);
	glBindVertexArray(0);
	// boundary wall done

	boundary_draw_hole(vp, 4.0f, c_white, head_position);
	//boundary_draw_hole(vp, 0.2f, c_red, hand_position[0]);
	//boundary_draw_hole(vp, 1.0f, c_green, hand_position[1]);
}
