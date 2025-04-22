#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aUV;
layout (location = 4) in vec3 aColor;

out VertexData {
	vec3 pos;
	vec2 UV;
	vec3 normal;
	vec3 color;
	vec3 tangent;
} data_out;

void main() {
	data_out.pos = aPos;
	data_out.UV = aUV;
	data_out.normal = aNormal;
	data_out.color = aColor;
	data_out.tangent = aTangent;
}