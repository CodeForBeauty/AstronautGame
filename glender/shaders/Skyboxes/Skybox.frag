#version 330 core

out vec4 FragColor;

in vec3 vUV;

uniform samplerCube u_Skybox;

void main() {
    FragColor = textureLod(u_Skybox, vUV, 4);
}