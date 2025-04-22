#version 330 core

in vec2 vUV;

const float BIAS = 0.0005f;

layout (std140) uniform Metrices {
    mat4 u_Proj;
	mat4 u_Cam;
	vec4 u_ViewPos;
};

uniform samplerCube u_ShadowMap;
uniform sampler2D u_ShadowRender;

uniform sampler2D uPosition;

uniform vec3 u_LightPos;
uniform float u_LightDistance;

out vec4 FragColor;

void main() {
	vec3 lightCoords = texture(uPosition, vUV).xyz - u_LightPos;
	vec2 screenSpace = vUV;

	float shadow = 1.0f;

	if (length(lightCoords) > u_LightDistance) {
		shadow = 1.0f + texture(u_ShadowRender, screenSpace).r;
		FragColor = vec4(shadow, shadow, shadow, 1.0f);
		return;
	}

	float closestDepth = 0.0f;
	float previous = texture(u_ShadowRender, screenSpace).r;
	float currentDepth = length(lightCoords) / u_LightDistance;

	int sampleRadius = 1;
	float pixelSize = 1.0f / textureSize(u_ShadowMap, 0).x;
	float tmpShadow = 0.0f;
	shadow = 0.0f;

	for (int y = -sampleRadius; y <= sampleRadius; y++) {
		for (int x = -sampleRadius; x <= sampleRadius; x++) {
			for (int z = -sampleRadius; z <= sampleRadius; z++) {
				closestDepth = texture(u_ShadowMap, lightCoords + vec3(x, y, z) * pixelSize).r;
				if (currentDepth > closestDepth + BIAS) {
					tmpShadow = previous + 0.4f;
				}
				else {
					tmpShadow = (1.0f + previous);
				}
				shadow += tmpShadow;
			}
		}
	}
	shadow /= pow((sampleRadius * 2 + 1), 3);

	FragColor = vec4(shadow, shadow, shadow, 1.0f);
}