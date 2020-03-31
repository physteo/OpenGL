#version 330 core

layout (location = 0) out vec4 o_Color;

uniform sampler2D u_Textures[20];

in vec4 out_Color;
in vec2 out_UV;
in vec4 out_TexUnit;

void main()
{
	if (out_TexUnit.x < 0.0)
	{
		o_Color = vec4(out_UV, 0.0, 1.0);//out_Color;
	}
	else
	{
		o_Color = texture(u_Textures[int(out_TexUnit.x)], out_UV);
	}
}