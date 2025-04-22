#version 330 core

layout (location = 0) in vec3 aPos;

layout (std140) uniform Metrices {
    mat4 u_Proj;
	mat4 u_Cam;
	vec4 u_ViewPos;
};

uniform mat4 u_Light;

out vec3 vWorldPos;
out vec3 vScreenPos;
out vec3 vLightPos;

void main() {
	gl_Position =  u_Proj * u_Cam * vec4(aPos, 1);

	vWorldPos = aPos;
	vScreenPos = gl_Position.xyz;
	vScreenPos /= gl_Position.w;

	vec4 lightPos = u_Light * vec4(aPos, 1);
	vLightPos = lightPos.xyz * lightPos.w;
}