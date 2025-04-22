#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 u_Light;

out vec4 vFragPos;

void main() {
	vFragPos = u_Light * vec4(aPos, 1);
	gl_Position = vFragPos;
}