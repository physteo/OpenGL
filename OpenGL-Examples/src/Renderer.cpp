#include "Renderer.h"

void InstanceSpecs::setBuffer(VertexArray& vertexArray, unsigned int buffer)
{
	// TODO: the following assumes the next vertex attrib is 2. It should instead retrieve the correct number
	// from the vao
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBindVertexArray(vertexArray.getID());
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceSpecs), (void*)offsetof(InstanceSpecs, modelMatrix));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceSpecs), (void*)offsetof(InstanceSpecs, color));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceSpecs), (void*)offsetof(InstanceSpecs, material));
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::init()
{
	// create instance buffer
	glCreateBuffers(1, &m_instancesBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_instancesBuffer);
	glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(InstanceSpecs), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

	if (maxTextureUnits < 3)
	{
		std::cout << "Must support more than 3 texture units." << std::endl;
		assert(false);
	}

	avgFlushBatchSize = 0.0f;
	// set opengl states
	
#ifdef TRICK
	clearTexUnits();
#endif
}

void Renderer::begin()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// clear data
	//m_toDraw.clear();
#ifdef TRICK
	clearTexUnits();
#else
	m_activeTexturesUnits.clear();
#endif
	
	numDrawCalls = 0;
	avgFlushBatchSize = 0.0f;
	glUseProgram(m_shader->GetRendererID());
}

void Renderer::end()
{
	avgFlushBatchSize = avgFlushBatchSize / float(numDrawCalls);
	glUseProgram(0);
}

void Renderer::submit(VertexArray vertexArray, Transform transform, Material material)
{
	InstanceSpecs is;
	is.modelMatrix = transform.position;
	is.material[0] = float(material.diffuse );
	is.material[1] = float(material.specular);
	is.material[2] = float(material.normal  );
	is.material[3] = material.specularity;

	submit(vertexArray, is);
}

struct TextureQuery
{
	unsigned int texID;
	bool found;
};

void Renderer::submit(VertexArray vertexArray, InstanceSpecs& instanceSpec)
{
	// Collect new textures
	TextureQuery newTexture[3] = { {0, true}, {0, true}, {0, true} };
	size_t texturesToAdd = 0;
	for (size_t i = 0; i < 3; ++i)
	{
		newTexture[i].texID = m_textureDatabase->operator[](int(instanceSpec.material[i]));
#ifdef TRICK
		int found = m_activeTexturesUnits[newTexture[i].texID];
		if (found < -0.5)
#else
		auto activeTexture = m_activeTexturesUnits.find(newTexture[i].texID);
		if (activeTexture == m_activeTexturesUnits.end())
#endif
		{
			newTexture[i].found = false;
			++texturesToAdd;
		}
	}

	// Flush if the maximum number of textures is surpassed
	bool flushed = false;
#ifdef TRICK
	if(numActiveTextureUnits + texturesToAdd > maxTextureUnits)
#else
	if (m_activeTexturesUnits.size() + texturesToAdd > maxTextureUnits)
#endif
	{
		flush();
		flushed = true;
	}

	// Register needed textures
	for (size_t i = 0; i < 3; ++i)
	{
		if (flushed == true)
		{
			// register all submitted textures
			registerTexture(newTexture[i].texID);
		}
		else
		{
			// register only new textures
			if (newTexture[i].found == false)
			{
				registerTexture(newTexture[i].texID);
			}
		}
	}

	// Is this mesh already registered?
	auto vaoData = m_toDraw.find(vertexArray.getID());
	if (vaoData == m_toDraw.end())
	{
		// No: register mesh
		auto inserted = m_toDraw.insert({ vertexArray.getID(), {} });
		if (inserted.second)
		{
			// Is this mesh at least known by the renderer?
			if (m_knownMeshes.find(vertexArray.getID()) == m_knownMeshes.end())
			{
				// No: set layout for that mesh  
				InstanceSpecs::setBuffer(vertexArray, m_instancesBuffer);
				m_knownMeshes.insert(vertexArray.getID());
			}

			vaoData = inserted.first;
		}
		else
		{
			GLCORE_ASSERT("Could not submit mesh (vao = {0}) to renderer.", vertexArray.getID());
		}
	}

	pushData(&vaoData->second, instanceSpec);
}

void Renderer::flush()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_instancesBuffer);
	for (auto it = m_toDraw.begin(); it != m_toDraw.end(); ++it)
	{
		const unsigned int& vaoID = it->first;
		std::vector<InstanceSpecs>& data = it->second;
		int numIndices = 6; // TODO: retrieve from VertexArray
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(InstanceSpecs), (void*)&data[0], GL_DYNAMIC_DRAW);
		glBindVertexArray(vaoID);
		glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0, data.size());
		++numDrawCalls;
		avgFlushBatchSize += data.size();
		glBindVertexArray(0);
		data.clear();
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef TRICK
	clearTexUnits();
#else
	m_activeTexturesUnits.clear();
#endif


}

void Renderer::registerTexture(unsigned int newTexID)
{
#ifdef TRICK
	int previousValue = m_activeTexturesUnits[newTexID];
	m_activeTexturesUnits[newTexID] = numActiveTextureUnits;
#else
	int numActiveTextureUnits = m_activeTexturesUnits.size();
	m_activeTexturesUnits.insert({ newTexID, numActiveTextureUnits});
#endif

	//x//glUseProgram(m_shader->GetRendererID());
	glActiveTexture(GL_TEXTURE0 + numActiveTextureUnits);
	glBindTexture(GL_TEXTURE_2D, newTexID);
	std::string uName = "u_Textures[" + std::to_string(numActiveTextureUnits) + "]";
	int location = glGetUniformLocation(m_shader->GetRendererID(), uName.c_str());
	glUniform1i(location, numActiveTextureUnits);
	//x//glUseProgram(0);
#ifdef TRICK
	if (m_activeTexturesUnits[newTexID] != previousValue)
	{
		++numActiveTextureUnits;
	}
#endif
}

void Renderer::pushData(std::vector<InstanceSpecs>* instanceVec, InstanceSpecs& instanceSpecs)
{
	instanceVec->push_back(instanceSpecs);

	for (size_t i = 0; i < 3; ++i)
	{
		unsigned int texID = m_textureDatabase->operator[](int(instanceSpecs.material[i]));
		instanceVec->back().material[i] = float(m_activeTexturesUnits[texID]);
	}
}

#ifdef TRICK
void Renderer::clearTexUnits()
{
	m_activeTexturesUnits.resize(10000);
	std::fill(m_activeTexturesUnits.begin(), m_activeTexturesUnits.end(), -1);
	numActiveTextureUnits = 0;
}
#endif
// recursive registering textures
//size_t texturesToAdd = 0;
//for (size_t i = 0; i < 3; ++i)
//{
//	unsigned int newTexID = m_textureDatabase->operator[](int(instanceSpec.material[i]));
//	
//	// Is the texture already registered?
//	auto activeTexture = m_activeTexturesUnits.find(newTexID);
//	if (activeTexture == m_activeTexturesUnits.end())
//	{
//		++texturesToAdd;
//		if (m_activeTexturesUnits.size() + texturesToAdd > maxTextureUnits)
//		{
//			flush();
//			submit(vertexArray, instanceSpec);
//			return;
//		}
//		registerTexture(newTexID);
//	}
//}