#version 320 es
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 uv;
layout(location = 2) uniform mat4 MVP;
layout(location = 3) uniform vec3 handPosition;
out vec2 st;
out vec4 vColor;
void main() {
    float nearHand = abs(distance(vertexPosition_modelspace, handPosition));
    if (nearHand < 11.0) {
        vColor = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        vColor = vec4(0.0, 0.0, 1.0, 1.0);
    }

    gl_Position =  MVP * vec4(vertexPosition_modelspace, 1.0);
    gl_PointSize = 10.0;

    st = uv;

}