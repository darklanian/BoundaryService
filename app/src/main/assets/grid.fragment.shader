#version 320 es
precision mediump float;
in vec2 st;
in vec4 vColor;
out vec4 color;
void main() {
    float line_width_half = 0.05;
    vec2 uv = st/5.0;
    vec2 mod = uv - floor(uv);
    if (st.y < 0.2) {
        color = vec4(0.0, 1.0, 0.0, 1.0); // floor - green
    } else if ( (mod.x < line_width_half || mod.x > (1.0-line_width_half)) || (mod.y < line_width_half || mod.y > (1.0-line_width_half)) )
        color = vColor;
    else
        color = vec4(0.0, 0.0, 0.0, 0.0); // otherwise - transparent

}