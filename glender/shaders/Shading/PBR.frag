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

uniform samplerCube u_Irradiance;
uniform samplerCube u_IrradianceSpec;
uniform sampler2D u_Brdf;

in vec2 vUV;

const float Pi = 3.14159265359f;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = Pi * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 LightReflectance(vec3 viewDir, vec3 normal, vec3 lightDir, vec3 albedo, float specular, float roughness, float metallic, vec3 F0) {
    vec3 H = normalize(viewDir + lightDir);

    float NDF = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), F0);
        
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 spec = (numerator / denominator) * specular;
    
    float NdotL = max(dot(normal, lightDir), 0.0);
    return (kD * albedo / Pi + spec) * NdotL;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 SceneIrradiance(vec3 albedo, vec3 normal, vec3 viewDir, float roughness, float metallic, vec3 F0) {
	vec3 kS = fresnelSchlickRoughness(max(dot(normal, viewDir), 0.0), F0, roughness); 
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;
	vec3 irradiance = texture(u_Irradiance, normal).rgb;

	vec3 R = reflect(-viewDir, normal);

	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(u_IrradianceSpec, R,  roughness * MAX_REFLECTION_LOD).rgb;
	vec2 envBRDF  = texture(u_Brdf, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

	return kD * irradiance * albedo + specular;
}

void main() {
	vec3 normal = normalize(vec3(texture(uNormal, vUV)));
	vec3 albedo = vec3(texture(uAlbedo, vUV));
	vec3 position = vec3(texture(uPosition, vUV));
	vec4 smra = texture(uSMRA, vUV);

	float specular = smra.x;
	float metallic = smra.y;
	float roughness = smra.z;
	float ao = smra.w;

	vec3 viewDir = normalize(vec3(u_ViewPos) - position);

	float ratio = 1.00 / 1.52;
	vec3 posDir = normalize(position - vec3(u_ViewPos));
	vec3 reflDir = reflect(posDir, normal);

	float shadow = texture(u_Shadow, vUV).r / (u_DirectCount + u_SpotCount + u_PointCount);

	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

	vec3 lighting = SceneIrradiance(albedo, normal, viewDir, roughness, metallic, F0);

	for (int i = 0; i < u_DirectCount; i++)	{
		vec3 lightDir = vec3(-u_Direct[i].Direction);
		vec3 lightColor = vec3(u_Direct[i].Color);
		vec3 diff = LightReflectance(viewDir, normal, normalize(lightDir), albedo, specular, roughness, metallic, F0);

		float dist = length(lightDir);
		float attenuation = u_Direct[i].Intensity / (dist * dist);
		vec3 radiance = lightColor * attenuation;

		lighting += diff * radiance * shadow;
	}
	for (int i = 0; i < u_PointCount; i++) {
		vec3 lightDir = vec3(u_Point[i].Position) - position;
		vec3 lightColor = vec3(u_Point[i].Color);
		vec3 diff = LightReflectance(viewDir, normal, normalize(lightDir), albedo, specular, roughness, metallic, F0);

		float dist = length(lightDir);
		float radius = u_Point[i].Distance;
		float intens = clamp(u_Point[i].Intensity - dist*dist/(radius*radius), 0.0, 1.0);
		intens *= intens;
		vec3 radiance = vec3(intens) * lightColor;

		lighting += diff * radiance * shadow;
	}
	for (int i = 0; i < u_SpotCount; i++) {
		vec3 lightDir = vec3(u_Spot[i].Position) - position;
		vec3 lightColor = vec3(u_Spot[i].Color);
		vec3 diff = LightReflectance(viewDir, normal, normalize(lightDir), albedo, specular, roughness, metallic, F0);

		float dist = length(lightDir);
		float radius = u_Point[i].Distance;
		float intens = clamp(u_Spot[i].Intensity - dist*dist/(radius*radius), 0.0, 1.0);
		intens *= intens;
		vec3 radiance = vec3(intens) * lightColor;

		float angle = dot(-vec3(u_Spot[i].Direction), normalize(lightDir));
		float cone = clamp((angle - u_Spot[i].OuterCone) / (u_Spot[i].InnerCone - u_Spot[i].OuterCone), 0.0f, 1.0f);

		lighting += diff * radiance * cone * shadow;
	}
	
	FragColor = vec4(lighting, 1);
}