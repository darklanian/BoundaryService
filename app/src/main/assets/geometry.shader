#version 320 es

layout (lines) in;
layout (line_strip, max_vertices = 2) out;
void main() {
    gl_Position = gl_in[0].gl_Position + vec4(0.0, 10.0, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    EndPrimitive();
}