#version 330 core

in vec2 vUV;

const float BIAS = 0.05f;

layout (std140) uniform Metrices {
    mat4 u_Proj;
	mat4 u_Cam;
	vec4 u_ViewPos;
};

uniform sampler2D u_ShadowMap;
uniform sampler2D u_ShadowRender;

uniform sampler2D uPosition;

uniform mat4 u_Light;

out vec4 FragColor;

void main() {
	vec4 lightPos = u_Light * vec4(texture(uPosition, vUV).xyz, 1);
	vec3 lightCoords = lightPos.xyz / lightPos.w;
	vec2 screenSpace = vUV;

	lightCoords = (lightCoords + 1.0f) / 2.0f;

	float shadow = 1.0f;
	if (lightCoords.z <= 1.0f) {
		float closestDepth = 0.0f;
		float previous = texture(u_ShadowRender, screenSpace).r;
		float currentDepth = lightCoords.z;

		int sampleRadius = 1;
		vec2 pixelSize = 1.0f / textureSize(u_ShadowMap, 0);
		float tmpShadow = 0.0f;
		shadow = 0.0f;

		for (int y = -sampleRadius; y <= sampleRadius; y++) {
			for (int x = -sampleRadius; x <= sampleRadius; x++) {
				closestDepth = texture(u_ShadowMap, lightCoords.xy + vec2(x, y) * pixelSize).r;
				if (currentDepth > closestDepth + BIAS) {
					tmpShadow = previous + 0.4f;
				}
				else {
					tmpShadow = (1.0f + previous);
				}
				shadow += tmpShadow;
			}
		}
		shadow /= pow((sampleRadius * 2 + 1), 2);
	}

	FragColor = vec4(shadow, shadow, shadow, 1.0f);
}