#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gAlbedo;
layout (location = 2) out vec3 gNormal;
layout (location = 3) out vec4 gSMRA;
layout (location = 4) out int gIndex;

uniform sampler2D u_AlbedoTex;
uniform vec4 u_Albedo;
uniform bool u_HasAlbedoTex;

uniform sampler2D u_NormalTex;
uniform bool u_HasNormalTex;

uniform sampler2D u_SpecularTex;
uniform float u_Specular;
uniform bool u_HasSpecularTex;

uniform sampler2D u_RoughnessTex;
uniform float u_Roughness;
uniform bool u_HasRoughnessTex;

uniform sampler2D u_MetalicTex;
uniform float u_Metalic;
uniform bool u_HasMetalicTex;

uniform sampler2D u_AOTex;
uniform bool u_HasAOTex;

uniform sampler2D u_Fontmap;
uniform vec2 u_FontmapSize;
uniform float u_FontWidth;

uniform int u_ObjectIndex;

in FragData {
	vec3 worldPos;
	vec2 screenPos;
	vec2 uv;
	mat3 tbn;
	vec3 normal;
} data_in;

void main() {
	float dscale = 1;
	vec2 pixelSize = dscale * (dFdx(data_in.uv) + dFdy(data_in.uv));
	vec2 spread = pixelSize * 1;
	float alpha = 0;
	int count = 0;
	for (float x = data_in.uv.x - spread.x; x < data_in.uv.x + spread.x; x+= pixelSize.x) {
		for (float y = data_in.uv.y - spread.y; y < data_in.uv.y + spread.y; y+= pixelSize.y) {
			float dist = texture(u_Fontmap, vec2(x, y)).r;
			alpha += smoothstep(0.5 + u_FontWidth - 0.06f, 0.5 + u_FontWidth + 0.0f, dist);
			count++;
		}
	}

	float dist = texture(u_Fontmap, data_in.uv).r;
	float mainAlpha = smoothstep(0.5 + u_FontWidth - 0.06f, 0.5 + u_FontWidth + 0.0, dist);

	alpha = (mainAlpha + alpha) / (count + 1);

	if (alpha <= 0.05f) {
		discard;
	}

	gPosition = data_in.worldPos;
	gAlbedo = u_HasAlbedoTex ? texture(u_AlbedoTex, data_in.uv) : u_Albedo;
	gAlbedo.a = alpha;
	gNormal = u_HasNormalTex ? data_in.tbn * (vec3(texture(u_NormalTex, data_in.uv)) * 2 - 1) : data_in.normal;
	
	gSMRA = vec4((u_HasSpecularTex ? texture(u_SpecularTex, data_in.uv).r : u_Specular),
				(u_HasMetalicTex ? texture(u_MetalicTex, data_in.uv).r : u_Metalic),
				(u_HasRoughnessTex ? texture(u_RoughnessTex, data_in.uv).r : u_Roughness),
				(u_HasAOTex ? texture(u_AOTex, data_in.uv).r : 1.0f));

	gIndex = u_ObjectIndex;
}