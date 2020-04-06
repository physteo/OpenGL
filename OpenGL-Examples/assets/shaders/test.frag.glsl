#version 330 core

layout (location = 0) out vec4 o_Color;
layout (location = 1) out vec4 o_Norm;

uniform sampler2D u_Textures[31];
uniform sampler2D u_ShadowMap;

in vec4 out_Color;
in vec2 out_UV;
in vec4 out_TexUnit;
in vec4 out_Position_LightSpace;
in vec3 out_LightDir_tan;
//in mat3 out_iTBN;

void main()
{
	vec3 normal_tan_color = texture(u_Textures[int(out_TexUnit.y)], out_UV).rgb;
	vec3 normal_tan = 2.0 * normal_tan_color - 1.0;
	//vec3 normal_world_color = (out_iTBN * normal_tan + 1.0) / 2.0;

	float diffuse = dot(normal_tan, -out_LightDir_tan);

	vec3 projCoords = out_Position_LightSpace.xyz / out_Position_LightSpace.w; // perspective adjustment
	projCoords = projCoords * 0.5 + 0.5; // normalized device coords
	
	float closestDepth = texture(u_ShadowMap, projCoords.xy).r;
	float bias = 0.01;
	float shadow = (projCoords.z - bias) < closestDepth ? 0.0 : 1.0;

	if (projCoords.z > 1.0)
		shadow = 0.0;

	if (out_TexUnit.x < 0.0)
	{
		o_Color = (1 - shadow) * vec4(out_UV, 0.0, 1.0);//out_Color;
	}
	else
	{
		vec3 diffBase = texture(u_Textures[int(out_TexUnit.x)], out_UV).rgb;
		float ambient = 0.5;
		vec3 ambColor  = ambient * diffBase;
		vec3 diffColor = diffuse * diffBase;

		o_Color = vec4( ambColor + (1.0 - shadow) * diffColor , 1.0);
		o_Norm = vec4(normal_tan_color, 1.0);
	}
}