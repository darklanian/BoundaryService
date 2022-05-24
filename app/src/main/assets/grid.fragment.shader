#version 320 es
precision mediump float;
layout(location = 3) uniform float proximity;
in vec2 st;
in vec4 vColor;
out vec4 color;
void main() {
    vec4 transparent = vec4(0.0, 0.0, 0.0, 0.0);
    float min_proximity = 10.0;
    float max_proximity = 15.0;
    float line_width_half = 0.05;
    vec2 uv = st/5.0;
    vec2 mod = uv - floor(uv);
    if (proximity > max_proximity) {
        color = transparent;
    } else if (st.y < 0.2) {
        color = vec4(0.0, 1.0, 0.0, 1.0);// floor - green
    } else if ((mod.x < line_width_half || mod.x > (1.0-line_width_half)) || (mod.y < line_width_half || mod.y > (1.0-line_width_half))) {
        float p = max(min_proximity, min(max_proximity, proximity)) - min_proximity;
        p = 0.3-p/(max_proximity-min_proximity)*0.3 + 0.2;
        if ((mod.x < p || mod.x >= 1.0-p) && (mod.y < p || mod.y >= 1.0-p)) {
            color = vColor;
        } else {
            color = transparent;
        }
    } else
        color = transparent;

}