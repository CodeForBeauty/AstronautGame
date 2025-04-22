#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 u_Sides[6];

out vec3 vFragPos;

void main()
{
	for (int face = 0; face < 6; ++face)
	{
		gl_Layer = face;
		for (int i = 0; i < 3; i++)
		{
			vFragPos = vec3(u_Sides[face] * gl_in[i].gl_Position);
			
			gl_Position = gl_in[i].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}