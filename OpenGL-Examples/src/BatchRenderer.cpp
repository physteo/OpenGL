#include "BatchRenderer.h"


void BatchRenderer::setMaxTextureUnits(int maxTextureUnitsIn)
{ 
	if (maxTextureUnitsIn != maxTextureUnits)
	{
		maxTextureUnitsChanged = true;
		maxTextureUnits = maxTextureUnitsIn;
	}
}

void BatchRenderer::init()
{
	// Create buffer for instances
	glCreateBuffers(1, &m_instancesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_instancesBuffer);
	glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(InstanceSpecs), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
	GLCORE_ASSERT(maxTextureUnits >= 3, "Must support more than 3 texture units.");

	avgFlushBatchSize = 0.0f;
}

void BatchRenderer::begin()
{
	// clear instance data, keep structure
	if (maxTextureUnitsChanged == false)
	{
		for (auto it = m_meshBatches.begin(); it != m_meshBatches.end(); ++it)
		{
			auto& texBatches = it->second;
			for (auto jt = texBatches.begin(); jt != texBatches.end(); ++jt)
			{
				jt->instanceData.clear();
			}
		}
	}
	else
	{
		for (auto it = m_meshBatches.begin(); it != m_meshBatches.end(); ++it)
		{
			it->second.clear();
		}
	}

	maxTextureUnitsChanged = false;
	numDrawCalls = 0;
	avgFlushBatchSize = 0.0f;
}

void BatchRenderer::end()
{
	avgFlushBatchSize = avgFlushBatchSize / float(numDrawCalls);
}

void BatchRenderer::submit(VertexArray vertexArray, Transform transform, Material material)
{
	InstanceSpecs is;
	is.modelMatrix = transform.model;
	//is.normalMatrix = glm::transpose(glm::inverse(transform.model));
	is.material[0] = float(material.diffuse);
	is.material[1] = float(material.specular);
	is.material[2] = float(material.normal);
	is.material[3] = material.specularity;

	submit(vertexArray, std::move(is) );
}

struct TextureQuery
{
	unsigned int texID;
	bool found;
};

void BatchRenderer::submit(VertexArray vertexArray, InstanceSpecs&& instanceSpec)
{
	glm::vec4 instanceTexUnits = instanceSpec.material;

	// Is this mesh already registered?
	auto meshBatch = m_meshBatches.find(vertexArray);
	if (meshBatch == m_meshBatches.end())
	{
		// No: register mesh
		auto inserted = m_meshBatches.insert({ vertexArray, {} });
		if (inserted.second)
		{
			// Is this mesh at least known by the renderer?
			if (m_knownMeshes.find(vertexArray.getID()) == m_knownMeshes.end())
			{
				// No: set layout for that mesh  
				InstanceSpecs::setBuffer(vertexArray, m_instancesBuffer);
				m_knownMeshes.insert(vertexArray.getID());
			}
	
			meshBatch = inserted.first;
		}
		else
		{
			GLCORE_ASSERT("Could not submit mesh (vao = {0}) to renderer.", vertexArray.getID());
		}
	}

	// Is this material already registered?
	// Assumption: the material is identified by the diffuse texture.
	TextureQuery newTexture = {0, false};
	newTexture.texID = m_textureDatabase->operator[](int(instanceTexUnits.x));
	
	// Search if this texture was already registered in some batch
	for (size_t i = 0; i < meshBatch->second.size(); ++i)
	{
		BatchData* texBatch = &meshBatch->second[i];
#ifdef TINYMAP_OPTIMIZATION
		if(texBatch->activeTexturesUnits.getValue(newTexture.texID) > -0.5)
#else
		auto found = texBatch->activeTexturesUnits.find(newTexture.texID);
		if (found != texBatch->activeTexturesUnits.end())
#endif
		{
			// found: push data into this batch
			pushInstance(texBatch, std::move(instanceSpec));
			return;
		}
	}

	// Texture was not found. Can I register it into the next texBatch?
	if (meshBatch->second.size() > 0)
	{
		if (meshBatch->second.back().activeTexturesUnits.size() + 3 > maxTextureUnits)
		{
			// No: need to create a new batch and register texture there
			meshBatch->second.push_back(BatchData{});
		}
	}
	else
	{
		meshBatch->second.push_back(BatchData{});
	}

	registerTexture(&meshBatch->second.back(), newTexture.texID);
	registerTexture(&meshBatch->second.back(), m_textureDatabase->operator[](int(instanceTexUnits.y)));
	registerTexture(&meshBatch->second.back(), m_textureDatabase->operator[](int(instanceTexUnits.z)));

	pushInstance(&meshBatch->second.back(), std::move(instanceSpec));
}

void BatchRenderer::draw(const GLCore::Utils::Shader& shader)
{
	glUseProgram(shader.GetRendererID());

	glBindBuffer(GL_ARRAY_BUFFER, m_instancesBuffer);
	for (auto it = m_meshBatches.begin(); it != m_meshBatches.end(); ++it)
	{
		const unsigned int& vaoID = it->first.getID();
		int numIndices = it->first.getNumIndices();
		glBindVertexArray(vaoID);

		std::vector<BatchData>& texBatches = it->second;
		for (auto jt = texBatches.begin(); jt != texBatches.end(); ++jt)
		{
			auto& instances = jt->instanceData;
#define PREV_PATH
#ifdef PREV_PATH
#ifdef TINYMAP_OPTIMIZATION
			//auto& textures = jt->activeTexturesUnits.getVec();
			for (size_t i = 0; i < jt->activeTexturesUnits.size(); ++i)
			{
				unsigned int texID = jt->activeTexturesUnits.getKey(i);
				int texUnit = jt->activeTexturesUnits.getValue(texID);
				std::pair<unsigned int, int> texture = { texID, texUnit };
#else
			auto& textures = jt->activeTexturesUnits;
			for (auto texture : textures)
			{
#endif
				glActiveTexture(GL_TEXTURE0 + texture.second);
				glBindTexture(GL_TEXTURE_2D, texture.first);
			}
#else
			auto& textures = jt->activeTexturesUnits;
			std::array<int, 32> textureSlots;
			size_t cnt = 0;
			for (const auto& texture : textures)
			{
				glActiveTexture(GL_TEXTURE0 + texture.second);
				glBindTexture(GL_TEXTURE_2D, texture.first);
				textureSlots[cnt] = cnt;
				++cnt;
			}
			int location = glGetUniformLocation(shader.GetRendererID(), "u_Textures");
			if(location >= 0)
				glUniform1iv(location, textures.size(), &textureSlots[0]);
#endif

			glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceSpecs), (void*)&instances[0], GL_DYNAMIC_DRAW);
			glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0, instances.size());
			glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceSpecs), NULL, GL_DYNAMIC_DRAW);
			++numDrawCalls;
			avgFlushBatchSize += instances.size();
		}
		glBindVertexArray(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glUseProgram(0);
}


void BatchRenderer::registerTexture(BatchData* batchData, unsigned int newTexID)
{
	int numActiveTextureUnits = batchData->activeTexturesUnits.size();
#ifdef TINYMAP_OPTIMIZATION
	batchData->activeTexturesUnits.insert({ newTexID, numActiveTextureUnits });
#else
	batchData->activeTexturesUnits.insert({ newTexID, numActiveTextureUnits});
#endif
}

void BatchRenderer::pushInstance(BatchData* batchData, InstanceSpecs&& instanceSpecs)
{
	batchData->instanceData.push_back(std::move(instanceSpecs));

	InstanceSpecs& batchBack = batchData->instanceData.back();

	float& mat_0 = batchBack.material[0];
	unsigned int texID = m_textureDatabase->operator[](int(mat_0));
	
#ifdef TINYMAP_OPTIMIZATION
	mat_0 = float(batchData->activeTexturesUnits.getValue(texID));
#else
	mat_0 = float(batchData->activeTexturesUnits[texID]);
#endif
	// Note: assumption made here, that textures within the same Material were
	// consecutively registered. This is for improving performance.
	batchBack.material[1] = mat_0 + 1;
	batchBack.material[2] = mat_0 + 2;
}
