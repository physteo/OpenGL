#version 330 core

layout (location = 0) out vec4 o_Color;

uniform sampler2D u_Textures[10];

in vec4 out_Color;
in vec2 out_UV;
in float out_TexID;

void main()
{
	if (out_TexID < 0.0)
	{
		o_Color = vec4(out_UV, 0.0, 1.0);//out_Color;
	}
	else
	{
		o_Color = texture(u_Textures[int(out_TexID)], out_UV);
	}
}