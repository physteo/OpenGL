#include "InstanceSpecs.h"

static void enable_vbo_instances(GLuint index, GLuint divisor, GLint numComponents, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, numComponents, type, normalized, stride, pointer);
	glVertexAttribDivisor(index, divisor);
}

void InstanceSpecs::setBuffer(VertexArray& vertexArray, unsigned int buffer)
{
	// TODO: the following assumes the next vertex attrib is 2. It should instead retrieve the correct number
	// from the vao
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBindVertexArray(vertexArray.getID());
	
	GLuint index = 3;
	enable_vbo_instances(index++, 1, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceSpecs), (void*)(offsetof(InstanceSpecs, modelMatrix) + 0 * sizeof(glm::vec4)));
	enable_vbo_instances(index++, 1, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceSpecs), (void*)(offsetof(InstanceSpecs, modelMatrix) + 1 * sizeof(glm::vec4)));
	enable_vbo_instances(index++, 1, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceSpecs), (void*)(offsetof(InstanceSpecs, modelMatrix) + 2 * sizeof(glm::vec4)));
	enable_vbo_instances(index++, 1, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceSpecs), (void*)(offsetof(InstanceSpecs, modelMatrix) + 3 * sizeof(glm::vec4)));

	enable_vbo_instances(index++, 1, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceSpecs), (void*)offsetof(InstanceSpecs, color));
	enable_vbo_instances(index++, 1, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceSpecs), (void*)offsetof(InstanceSpecs, material));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}