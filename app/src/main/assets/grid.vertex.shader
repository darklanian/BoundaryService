#version 320 es
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 uv;
layout(location = 2) uniform mat4 MVP;
layout(location = 3) uniform float dist_to_head;
out vec2 st;
out vec4 vColor;
void main() {
    float a = 0.0;
    if (dist_to_head < 2.0) {
        a = 1.0;
    } else if (dist_to_head < 10.0) {
        a = 2.0/dist_to_head;
    }
    vColor = vec4(0.0, 0.0, 1.0, a);

    gl_Position =  MVP * vec4(vertexPosition_modelspace, 1.0);
    gl_PointSize = 10.0;

    st = uv;
}