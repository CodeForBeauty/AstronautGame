#version 330 core

out vec4 FragColor;

uniform sampler2D u_Color;

in vec2 vUV;

const float gamma = 2.2f;

void main() {
	vec4 color = texture(u_Color, vUV);

	color.rgb = pow(color.rgb, vec3(1 / gamma));

	FragColor = color;
}