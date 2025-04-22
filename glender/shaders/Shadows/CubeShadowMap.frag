#version 330 core

in vec4 vFragPos;

uniform vec3 u_LightPos;
uniform float u_LightDistance;

void main() {
	gl_FragDepth = length(u_LightPos - vFragPos.xyz) / u_LightDistance;
}