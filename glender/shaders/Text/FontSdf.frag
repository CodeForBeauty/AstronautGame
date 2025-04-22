#version 330 core

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D u_Glyph;

uniform vec2 u_TexSize;

void main() {
    vec2 uv = vUV;
    uv.y = 1 - uv.y;

    vec2 stepSize = vec2(1.0f / u_TexSize.x, 1.0f / u_TexSize.y);
    vec2 resolution = textureSize(u_Glyph, 0);
    stepSize = vec2(1.0f / resolution.x, 1.0f / resolution.y);
    vec2 resSqrd = u_TexSize * u_TexSize;
    float closestDistance = resSqrd.x + resSqrd.y;

    int sampleSize = int((u_TexSize.x + u_TexSize.y) / 11);
    vec2 spread = stepSize * sampleSize;
    vec2 maxDis = spread * spread * 4;
    maxDis *= maxDis;
    closestDistance = maxDis.x + maxDis.y;

    float current = texture(u_Glyph, uv).r;
    for (float x = uv.x - spread.x; x < uv.x + spread.x; x += stepSize.x) {
        for (float y = uv.y - spread.y; y < uv.y + spread.y; y += stepSize.y) {
            if (current != texture(u_Glyph, vec2(x, y)).r) {
                vec2 sub = uv - vec2(x, y);
                sub *= sub;
                float dist = sub.x + sub.y;
                if (dist < closestDistance) {
                    closestDistance = dist;
                }
            }
        }
    }

    closestDistance /= maxDis.x + maxDis.y;

    FragColor = vec4(0.5f + 0.5f * (current == 0 ? -1 : 1) * sqrt(closestDistance));
}