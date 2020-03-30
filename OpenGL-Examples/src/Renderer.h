#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

#include "Transform.h"
#include "Material.h"

struct VertexArray
{
	inline void setID(unsigned int id) { m_id = id; }
	inline unsigned int getID() const { return m_id; }
private:
	unsigned int m_id;
	int numIndices;
};

struct VertexArrayHasher
{
	using argument_type = VertexArray;
	using result_type = unsigned long long;

	result_type operator()(const argument_type& f) const
	{
		return std::hash<unsigned long long>()(f.getID());
	}
};

struct InstanceSpecs
{
	glm::vec4 modelMatrix;
	glm::vec4 color;
	float texUnit;

	static void setBuffer(VertexArray& mesh, unsigned int buffer);
};

class Renderer
{
public:

	// To be used once (not every frame). Creates the buffer on the GPU for 
	// storing data of instances. Retrieves the maximum number of texture units.
	void init();
	
	// Use before submitting renderables. Clears color and depth buffer.
	// Clears the data to be drawn and active textures. Binds the shader.
	void begin();
	
	// Use after the last flush. Unbinds the shader.
	void end();

	// Submit a mesh to be rendered at a transform with a material. 
	void submit(VertexArray vertexArray, Transform transform, Material material);

	// Issues all the draw calls collected with submit.
	void flush();

	void setShader(GLCore::Utils::Shader* shader) { m_shader = shader; }
	void setTextures(std::vector<unsigned int>* textureID) { m_textureDatabase = textureID; }
	inline unsigned getNumDrawCalls() const { return numDrawCalls; }
	inline void setMaxTextureUnits(int maxTextureUnitsIn) { maxTextureUnits = maxTextureUnitsIn; }

private:
	void submit(VertexArray mesh, InstanceSpecs& instanceSpec);
	void registerTexture(unsigned int newTexID);
	void pushData(std::vector<InstanceSpecs>* instanceVec, InstanceSpecs& instanceSpecs);

	GLCore::Utils::Shader* m_shader;
	std::vector<unsigned int>* m_textureDatabase;
	std::unordered_set<unsigned int> m_knownMeshes;

	std::unordered_map<unsigned int, int> m_activeTextures;
	std::unordered_map<unsigned int, std::vector<InstanceSpecs>> m_toDraw;

	unsigned int m_instancesBuffer;
	int maxTextureUnits;

	// stats
	unsigned numDrawCalls;
};