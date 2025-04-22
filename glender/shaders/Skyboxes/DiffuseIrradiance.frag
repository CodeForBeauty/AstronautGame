#version 330 core

out vec4 FragColor;

in vec3 vFragPos;

uniform samplerCube u_Skybox;

const float PI = 3.14159265359;

void main() {
    vec3 normal = normalize(vFragPos);
  
    vec3 irradiance = vec3(0.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    const float sampleDelta = 0.05;
    float nrSamples = 0.0; 
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += texture(u_Skybox, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    FragColor = vec4(irradiance, 1.0);
}