#version 320 es
precision mediump float;
uniform vec4 vColor;
out vec4 color;
void main() {
    color = vColor;
}