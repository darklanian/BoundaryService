#version 320 es
precision mediump float;
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 uv;
layout(location = 2) uniform mat4 MVP;
layout(location = 3) uniform float proximity;
out vec2 st;
void main() {
    gl_Position =  MVP * vec4(vertexPosition_modelspace, 1.0);
    gl_PointSize = 10.0;

    st = uv;
}