#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_UV;
layout (location = 3) in mat4 a_Model;
layout (location = 7) in vec4 a_Color;
layout (location = 8) in vec4 a_TexUnit;

uniform mat4 u_ViewProjection;
uniform mat4 u_LightViewProjection;
uniform vec3 u_LightDir;

out vec4 out_Position_LightSpace;
out vec4 out_Color;
out vec2 out_UV;
out vec4 out_TexUnit;
out vec3 out_LightDir_tan;
//out mat3 out_iTBN;

vec3 compute_tangent(vec3 normal)
{
	if (abs(normal.x) >= 0.57735027)
	{
		return normalize(vec3(normal.y, -normal.x, 0.0 ));
	}
	else
	{
		return normalize(vec3(0.0, normal.z, -normal.y ));
	}
}

void main()
{
	vec4 positionWorld = a_Model * vec4(a_Position, 1.0f);//a_Position / a_Displace.w + a_Displace.xyz;
	gl_Position = u_ViewProjection * positionWorld;
	out_Position_LightSpace = u_LightViewProjection * positionWorld;

	// TBN matrix
	vec3 normal = normalize(a_Normal);
	vec3 tangent = compute_tangent(normal);
	vec3 bitangent = cross(normal, tangent);
	mat3 iTBN = transpose(mat3(bitangent, tangent, normal));
	//out_iTBN = iTBN;

	out_LightDir_tan = iTBN * u_LightDir;
	out_Color = a_Color;
	out_TexUnit = a_TexUnit;
	out_UV = a_UV;
}