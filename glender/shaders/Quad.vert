#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 3) in vec2 aUV;

out vec2 vUV;

void main() {
	gl_Position = vec4(aPos, 1);
	//data_out.uv = aUV;
	vUV = aUV;
}