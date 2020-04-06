#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

#include "Renderer.h"

#include <map>
#include "InstanceSpecs.h"

//#define TINYMAP_OPTIMIZATION
#ifdef TINYMAP_OPTIMIZATION
#include "TinyMap.h"
#endif


// Batches instances to be drawn according to their geometry (VertexArray) and 
// their textures (Material). The shader 
class BatchRenderer : public Renderer
{
public:
	// To be used once (not every frame). Creates the buffer on the GPU for 
	// storing the data of instances to be drawn. Retrieves the maximum 
	// number of texture units.
	virtual void init() override;

	// Clears the data to be drawn and active textures. Reset statistics.
	// Use before the first submit command.
	virtual void begin() override;

	// Finishes calculating statistics of batching.
	virtual void end() override;

	// Submits a mesh to be rendered at a transform with a material. 
	virtual void submit(VertexArray vertexArray, Transform transform, Material material) override;
	
	// Issues all the draw calls collected with submit using a shader.
	virtual void draw(const GLCore::Utils::Shader& shader) override;

	inline void setTextures(std::vector<unsigned int>* textureID) { m_textureDatabase = textureID; }

	inline unsigned getNumDrawCalls() const { return numDrawCalls; }
	inline float getAvgFlushBatchSize() const { return avgFlushBatchSize; }
	void setMaxTextureUnits(int maxTextureUnitsIn);
	inline int getMaxTextureUnits() const { return maxTextureUnits; }

private:
	// Holds a list of instances to be drawn with a set of textures.
	struct BatchData
	{
		std::vector<InstanceSpecs> instanceData;
#ifdef TINYMAP_OPTIMIZATION
		TinyMap activeTexturesUnits;
#else
		std::unordered_map<unsigned int, int> activeTexturesUnits;
#endif
		void clear()
		{
			activeTexturesUnits.clear();
			instanceData.clear();
		}
	};

	
	// Data is batched first according to the mesh (VertexArray), then
	// according to the textures (it is likely that a program would 
	// use different textures for the same mesh).
	std::unordered_map<VertexArray, std::vector<BatchData>, VertexArrayHasher > m_meshBatches;

	// A VertexArray needs to be supplied with additional buffers where
	// data of the instances are stored. We do this only once per VertexArray
	// and keep track of those we 
	std::unordered_set<unsigned int> m_knownMeshes;

	void submit(VertexArray mesh, InstanceSpecs&& instanceSpec);
	void registerTexture(BatchData* batchData, unsigned int newTexID);
	void pushInstance(BatchData* batchData, InstanceSpecs&& instanceSpecs);

	std::vector<unsigned int>* m_textureDatabase;

	unsigned int m_instancesBuffer;
	int maxTextureUnits;
	bool maxTextureUnitsChanged = false;

	// stats
	unsigned int numDrawCalls;
	float avgFlushBatchSize;
};