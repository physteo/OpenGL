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
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceSpecs), (void*)offsetof(InstanceSpecs, texUnit));
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

	//TODO@ restore this
	//glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
	maxTextureUnits = 5;

	// set opengl states
}

void Renderer::begin()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_toDraw.clear();
	m_activeTextures.clear();
	glUseProgram(m_shader->GetRendererID());
	numDrawCalls = 0;
}

void Renderer::end()
{
	glUseProgram(0);
}

void Renderer::submit(VertexArray vertexArray, Transform transform, Material material)
{
	InstanceSpecs is;
	is.modelMatrix = transform.position;
	is.texUnit = material.diffuse;
	submit(vertexArray, is);
}

void Renderer::submit(VertexArray vertexArray, InstanceSpecs& instanceSpec)
{
	unsigned int newTexID = m_textureDatabase->operator[](int(instanceSpec.texUnit));

	// Is the texture already registered?
	auto activeTexture = m_activeTextures.find(newTexID);
	if (activeTexture == m_activeTextures.end())
	{
		if (m_activeTextures.size() >= maxTextureUnits)
		{
			flush();
		}
		registerTexture(newTexID);
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

	vaoData = m_toDraw.find(vertexArray.getID()); //TODO: remove this?
	pushData(&vaoData->second, instanceSpec);

}

void Renderer::flush()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_instancesBuffer);
	for (auto vaoData : m_toDraw)
	{
		const unsigned int& vaoID = vaoData.first;
		std::vector<InstanceSpecs>& data = vaoData.second;
		int numIndices = 6; // TODO: retrieve from VertexArray
		GLsizei a;
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(InstanceSpecs), (void*)&data[0], GL_DYNAMIC_DRAW);
		glBindVertexArray(vaoID);
		glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0, data.size());
		++numDrawCalls;
		glBindVertexArray(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_toDraw.clear();
	m_activeTextures.clear();
}

void Renderer::registerTexture(unsigned int newTexID)
{
	int numActiveTextureUnits = m_activeTextures.size();
	m_activeTextures.insert({ newTexID, numActiveTextureUnits});
	//x//glUseProgram(m_shader->GetRendererID());
	glActiveTexture(GL_TEXTURE0 + numActiveTextureUnits);
	glBindTexture(GL_TEXTURE_2D, newTexID);
	std::string uName = "u_Textures[" + std::to_string(numActiveTextureUnits) + "]";
	int location = glGetUniformLocation(m_shader->GetRendererID(), uName.c_str());
	glUniform1i(location, numActiveTextureUnits);
	//x//glUseProgram(0);
}

void Renderer::pushData(std::vector<InstanceSpecs>* instanceVec, InstanceSpecs& instanceSpecs)
{
	instanceVec->push_back(instanceSpecs);

	unsigned int newTexID = m_textureDatabase->operator[](int(instanceSpecs.texUnit));
	instanceVec->back().texUnit = m_activeTextures[newTexID];
}