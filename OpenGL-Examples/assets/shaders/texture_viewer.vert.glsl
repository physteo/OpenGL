#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_UV;

out vec2 out_UV;

void main()
{
	gl_Position = vec4(2.0 * a_Position, 1.0);
	out_UV = a_UV;
}