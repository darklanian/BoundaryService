#version 320 es
precision mediump float;
in vec2 UV;
out vec4 color;
void main() {
    vec2 uv = UV/5.0;
    vec2 mod = uv - floor(uv);
    if (UV.y < 0.2) {
        color = vec4(0.0, 1.0, 0.0, 1.0);
    } else if ( (mod.x < 0.05 || mod.x > 0.95) || (mod.y < 0.05 || mod.y > 0.95) )
        color = vec4(1.0, 0.0, 0.0, 1.0);
    else
        color = vec4(0.0, 0.0, 0.0, 0.0);

}