#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_UV;
layout (location = 2) in vec4 a_Displace;
layout (location = 3) in vec4 a_Color;
layout (location = 4) in float a_TexID;

uniform mat4 u_ViewProjection;

out vec4 out_Color;
out vec2 out_UV;
out float out_TexID;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_Position / a_Displace.w + a_Displace.xyz, 1.0f);
	out_Color = a_Color;
	out_TexID = a_TexID;
	out_UV = a_UV;
}