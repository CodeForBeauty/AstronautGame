#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aUV;
layout (location = 4) in vec3 aColor;
layout (location = 5) in ivec4 aBoneIndices;
layout (location = 6) in vec4 aBoneWeights;

const int MAX_BONES = 150;//{MAX_BONES};
uniform mat4 u_Bones[MAX_BONES];

out VertexData {
	vec3 pos;
	vec2 UV;
	vec3 normal;
	vec3 color;
	vec3 tangent;
} data_out;

void main() {
	mat4 boneMatrix = mat4(0.0f);

	int bonesInfluence = 0;

	for (int i = 0; i < 4; i++) {
		if (aBoneIndices[i] < 0 || aBoneWeights[i] <= 0) {
			continue;
		}

		boneMatrix += u_Bones[aBoneIndices[i]] * aBoneWeights[i];
		bonesInfluence++;
	}
	
	if (bonesInfluence > 0) {
		mat3 boneRotation = mat3(boneMatrix);
		data_out.pos = vec3(boneMatrix * vec4(aPos, 1));
		data_out.normal = boneRotation * aNormal;
		data_out.tangent = boneRotation * aTangent;
	}
	else {
		data_out.pos = aPos;
		data_out.normal = aNormal;
		data_out.tangent = aTangent;
	}

	data_out.UV = aUV;
	data_out.color = aColor;
}