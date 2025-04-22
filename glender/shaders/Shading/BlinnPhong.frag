#version 330 core

out vec4 FragColor;

const int DIRECT_LIGHTS = 8;
const int POINT_LIGHTS = 100;
const int SPOT_LIGHTS = 50;

struct DirectLight {
    vec4 Color;
    vec4 Direction;
	float Intensity;
};
struct PointLight {
	vec4 Color;
	vec4 Position;
	float Intensity;
	float Distance;
};
struct SpotLight {
	vec4 Color;
	vec4 Position;
	vec4 Direction;
	float Intensity;
	float InnerCone;
	float OuterCone;
	float Distance;
};

layout (std140) uniform Lights {
    DirectLight u_Direct[DIRECT_LIGHTS];
	PointLight u_Point[POINT_LIGHTS];
	SpotLight u_Spot[SPOT_LIGHTS];

	int u_DirectCount;
	int u_PointCount;
	int u_SpotCount;
};

layout (std140) uniform Metrices {
    mat4 u_Proj;
	mat4 u_Cam;
	vec4 u_ViewPos;
};

uniform sampler2D uPosition;
uniform sampler2D uAlbedo;
uniform sampler2D uNormal;
uniform sampler2D uSMRA;

uniform sampler2D u_Shadow;

uniform samplerCube u_Skybox;

in vec2 vUV;

void main() {
	vec3 normal = normalize(vec3(texture(uNormal, vUV)));
	vec3 albedo = vec3(texture(uAlbedo, vUV));
	vec3 position = vec3(texture(uPosition, vUV));
	vec4 smra = texture(uSMRA, vUV);

	float specular = smra.x;
	float metalic = smra.y;
	float roughness = smra.z;
	float ao = smra.w;

	vec3 viewDir = normalize(vec3(u_ViewPos) - position);

	float ratio = 1.00 / 1.52;
	vec3 posDir = normalize(position - vec3(u_ViewPos));
	vec3 reflDir = refract(posDir, normal, ratio);

	vec3 lighting = albedo * 0.3f * texture(u_Skybox, reflDir).rgb;

	float shadow = texture(u_Shadow, vUV).r / (u_DirectCount + u_SpotCount + u_PointCount);

	for (int i = 0; i < u_DirectCount; i++)	{
		float diff = max(dot(normal, -vec3(u_Direct[i].Direction)), 0.0);

		vec3 reflectDir = reflect(vec3(u_Direct[i].Direction), normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular1 = specular * spec * vec3(u_Direct[i].Color);

		vec3 diffuse = (diff * vec3(u_Direct[i].Color) + specular1) * albedo * shadow;
		lighting += diffuse;
	}
	for (int i = 0; i < u_PointCount; i++) {
		vec3 lightDir = normalize(vec3(u_Point[i].Position) - position);
		float diff = max(dot(normal, lightDir), 0.0);

		vec3 reflectDir = reflect(-lightDir, normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular1 = specular * spec * vec3(u_Point[i].Color);

		float dist = length(vec3(u_Point[i].Position) - position);
		float a = 0.1 / u_Point[i].Distance;
		float b = 0.1;
		float intens = u_Point[i].Intensity / (a * (dist * dist) + b * dist + 1.0f);
		vec3 radiance = vec3(intens);

		vec3 diffuse = (diff * vec3(u_Point[i].Color) + specular1) * albedo * radiance * shadow;
		lighting += diffuse;
	}
	for (int i = 0; i < u_SpotCount; i++) {
		vec3 lightDir = normalize(vec3(u_Spot[i].Position) - position);
		float diff = max(dot(normal, lightDir), 0.0);

		vec3 reflectDir = reflect(-lightDir, normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular1 = specular * spec * vec3(u_Spot[i].Color);

		float dist = length(vec3(u_Spot[i].Position) - position);
		float a = 0.1 / u_Spot[i].Distance;
		float b = 0.1;
		float intens = u_Spot[i].Intensity / (a * (dist * dist) + b * dist + 1.0f);
		vec3 radiance = vec3(intens);

		float angle = dot(-vec3(u_Spot[i].Direction), lightDir);
		float cone = clamp((angle - u_Spot[i].OuterCone) / (u_Spot[i].InnerCone - u_Spot[i].OuterCone), 0.0f, 1.0f);

		vec3 diffuse = (diff * vec3(u_Spot[i].Color) + specular1) * albedo * radiance * cone * shadow;
		lighting += diffuse;
	}
	
	FragColor = vec4(lighting, 1);
}