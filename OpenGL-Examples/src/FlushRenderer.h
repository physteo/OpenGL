#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

#include "Renderer.h"

#include <map>
#include "InstanceSpecs.h"

//#define TRICK

class FlushRenderer : public Renderer
{
public:
	// To be used once (not every frame). Creates the buffer on the GPU for 
	// storing data of instances. Retrieves the maximum number of texture units.
	virtual void init() override;
	
	// Use before submitting renderables. Clears color and depth buffer.
	// Clears the data to be drawn and active textures. Binds the shader.
	virtual void begin() override;
	
	// Use after the last flush. Unbinds the shader.
	virtual void end() override;

	// Submit a mesh to be rendered at a transform with a material. 
	virtual void submit(VertexArray vertexArray, Transform transform, Material material) override;

	// Issues all the draw calls collected with submit.
	virtual void draw(const GLCore::Utils::Shader& shader) override;
	
	// Issues draw calls collected with submit and empties the data.
	void flush();

	inline void setShader(GLCore::Utils::Shader* shader) { m_shader = shader; }
	inline void setTextures(std::vector<unsigned int>* textureID) { m_textureDatabase = textureID; }
	inline unsigned getNumDrawCalls() const { return numDrawCalls; }
	inline float getAvgFlushBatchSize() const { return avgFlushBatchSize; }
	inline void setMaxTextureUnits(int maxTextureUnitsIn) { maxTextureUnits = maxTextureUnitsIn; }

private:
	void submit(VertexArray mesh, InstanceSpecs& instanceSpec);
	void registerTexture(unsigned int newTexID);
	void pushInstance(std::vector<InstanceSpecs>* instanceVec, InstanceSpecs& instanceSpecs);

	GLCore::Utils::Shader* m_shader;
	std::vector<unsigned int>* m_textureDatabase;
	std::unordered_set<unsigned int> m_knownMeshes;

	
#ifdef TRICK
	std::vector<int> m_activeTexturesUnits;
	unsigned int numActiveTextureUnits;
	void clearTexUnits();
#else
	std::unordered_map<unsigned int, int> m_activeTexturesUnits;
#endif
	std::unordered_map<VertexArray, std::vector<InstanceSpecs>, VertexArrayHasher> m_meshBatches;

	unsigned int m_instancesBuffer;
	int maxTextureUnits;

	// stats
	unsigned numDrawCalls;
	float avgFlushBatchSize;
};