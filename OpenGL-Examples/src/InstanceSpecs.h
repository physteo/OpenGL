#pragma once

#include "VertexArray.h"

struct InstanceSpecs
{
	glm::mat4 modelMatrix;
	//glm::mat4 normalMatrix;
	glm::vec4 color;
	glm::vec4 material;

	static void setBuffer(VertexArray& mesh, unsigned int buffer);
};
