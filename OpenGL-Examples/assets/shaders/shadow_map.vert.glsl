#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 3) in mat4 a_Model;

uniform mat4 u_LightViewProjection;

void main()
{
	//gl_Position = u_LightViewProjection * vec4(a_Position / a_Displace.w + a_Displace.xyz, 1.0f);
	gl_Position = u_LightViewProjection * a_Model * vec4(a_Position, 1.0f);
}