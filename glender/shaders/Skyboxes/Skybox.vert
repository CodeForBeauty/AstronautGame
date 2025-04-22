#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 vUV;

layout (std140) uniform Metrices {
    mat4 u_Proj;
	mat4 u_Cam;
	vec4 u_ViewPos;
};

void main() {
    vUV = aPos;
    
    vec4 pos = u_Proj * mat4(mat3(u_Cam)) * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
    gl_Position.z -= 0.00001f;
}  