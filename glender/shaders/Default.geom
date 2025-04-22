#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout (std140) uniform Metrices {
    mat4 u_Proj;
	mat4 u_Cam;
	vec4 u_ViewPos;
};

uniform mat4 u_Transform;

in VertexData {
	vec3 pos;
	vec2 UV;
	vec3 normal;
	vec3 color;
	vec3 tangent;
} data_in[];

out FragData {
	vec3 worldPos;
	vec2 screenPos;
	vec2 uv;
	mat3 tbn;
	vec3 normal;
} data_out;

void main() {
	for (int i = 0; i < 3; i++) {
		gl_Position =  u_Proj * u_Cam * u_Transform * vec4(data_in[i].pos, 1);

		data_out.worldPos = vec3(u_Transform * vec4(data_in[i].pos, 1));
		data_out.uv = data_in[i].UV;

		data_out.screenPos = (vec2(gl_Position) / gl_Position.w + 1) / 2;

		mat3 M = mat3(u_Transform);
		vec3 N = normalize(M * data_in[i].normal);
		data_out.normal = N;

		vec3 T = normalize(data_in[i].tangent);
		T = normalize(T - dot(T, N) * N);
		vec3 B = normalize(cross(T, N));
		mat3 TBN = mat3(T, B, N);
		data_out.tbn = TBN;

		EmitVertex();
	}
	EndPrimitive();
}