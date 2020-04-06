#version 330 core

in vec2 out_UV;

uniform sampler2D u_Texture;

out vec4 color;

void main()
{
	color = texture(u_Texture, out_UV);
	//color = vec4(outUV, 0.0, 1.0);
}