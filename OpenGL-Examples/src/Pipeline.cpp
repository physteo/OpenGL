#include "Pipeline.h"

#include <GLCore.h>
#include <GLCoreUtils.h>

Pipeline::~Pipeline()
{
	// clear textures
	glDeleteTextures(m_TextureID.size(), &m_TextureID[0]);
	glDeleteTextures(2, m_TextureAttachment);

	// clear FBOs
	glDeleteFramebuffers(1, &m_FboID);

	// clear shaders
	delete m_ShadowMapShader;
	delete m_TextureViewShader;
	delete m_Shader;

	// clear geometry
	glDeleteVertexArrays(1, &m_QuadVAO.id);
	glDeleteVertexArrays(1, &m_CircleVAO.id);
	glDeleteVertexArrays(1, &m_TriangleVAO.id);
	glDeleteBuffers(1, &m_QuadVB);
	glDeleteBuffers(1, &m_QuadIB);
	glDeleteBuffers(1, &m_CircleVB);
	glDeleteBuffers(1, &m_CircleIB);
	glDeleteBuffers(1, &m_TriangleVB);
	glDeleteBuffers(1, &m_TriangleIB);
}

void Pipeline::init()
{
	// OpenGL states
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	initFramebuffers();
	initShaders();
	initVertexArrays();

	m_batchRenderer.init();
	setMaxTextureUnits(16);
	m_batchRenderer.setTextures(&m_TextureID);
}

void Pipeline::update()
{
	using namespace GLCore;
	using namespace GLCore::Utils;

	Window* win = &Application::Get().GetWindow();

	// Submit objects to renderer
	m_batchRenderer.begin();
	for (auto& instance : *m_objects)
	{
		Transform tr;
		tr.model = instance.model;

		Material mat;
		mat.diffuse = instance.texUnitDiff;
		mat.specular = instance.texUnitSpec;
		mat.normal = instance.texUnitNorm;
		mat.specularity = instance.specularity;
		m_batchRenderer.submit(instance.vao, tr, mat);
	}

	// Draw shadow map 
	glm::mat4 lightViewProjection =
		m_camera->GetProjectionMatrix() * glm::lookAt(m_camera->GetPosition() - m_sunDir, m_camera->GetPosition(), glm::vec3{ 0.0, 1.0, 0.0 });
	{
		{
			glUseProgram(m_ShadowMapShader->GetRendererID());
			int location = glGetUniformLocation(m_ShadowMapShader->GetRendererID(), "u_LightViewProjection");
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(lightViewProjection));
		}

		glBindFramebuffer(GL_FRAMEBUFFER, m_FboIDShadow);
		glViewport(0, 0, 1024, 1024);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_batchRenderer.draw(*m_ShadowMapShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Draw actual objects
	{
		glUseProgram(m_Shader->GetRendererID());
		{
			int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewProjection");
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_camera->GetViewProjectionMatrix()));
		}
		{
			int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_LightViewProjection");
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(lightViewProjection));
		}
		{
			int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_LightDir");
			glUniform3f(location, m_sunDir.x, m_sunDir.y, m_sunDir.z);
		}
		{
			std::pair<unsigned int, int> texture = {m_TextureAttachmentShadowFbo, 31};
			glActiveTexture(GL_TEXTURE0 + texture.second);
			glBindTexture(GL_TEXTURE_2D, texture.first);
			int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ShadowMap");
			glUniform1i(location, texture.second);
		}
	
		glBindFramebuffer(GL_FRAMEBUFFER, m_FboID);
		glViewport(0, 0, win->GetWidth(), win->GetHeight());
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_batchRenderer.draw(*m_Shader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	m_batchRenderer.end();

	// draw texture from previous step to default fbo
	{
		glViewport(0, 0, win->GetWidth(), win->GetHeight());
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(m_TextureViewShader->GetRendererID());

		// passing the texture
		{
			std::string uName = "u_Texture";
			int location = glGetUniformLocation(m_TextureViewShader->GetRendererID(), uName.c_str());
			glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, m_TextureAttachment[m_attachment]);
			//glBindTexture(GL_TEXTURE_2D, m_attachment == 1 ? m_TextureAttachmentShadowFbo : m_TextureAttachment[0]);
			glBindTexture(GL_TEXTURE_2D, m_attachment == 1 ? m_GBuffer.norm : m_TextureAttachment[0]);
			glUniform1i(location, 0);
			GLCORE_ASSERT(location >= 0, "Uniform {0} does not exist in the shader.", uName)
		}

		// draw call
		glBindVertexArray(m_QuadVAO.getID());
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		glUseProgram(0);
	}
}

void PipelineImGuiController::update()
{
	ImGui::Begin("Pipeline controller");

	ImGui::SliderInt("Output tex", &(m_pipeline->m_attachment), 0, 2);
	static glm::vec3 sunDir = { 0.0f, -0.13f, -1.0f };
	if (ImGui::SliderFloat3("Sun dir", &sunDir[0], -2.0, 2.0))
	{
		m_pipeline->m_sunDir = glm::normalize(sunDir);
	}

	static int maxTexUnit = m_pipeline->getMaxTextureUnits();
	if (ImGui::SliderInt("Max Texture Units", &maxTexUnit, 1, 64))
	{
		m_pipeline->setMaxTextureUnits(maxTexUnit);
	}

	ImGui::End();
}

void Pipeline::setMaxTextureUnits(unsigned int maxTexUnit)
{
	m_batchRenderer.setMaxTextureUnits(maxTexUnit);
	
	std::array<int, 64> textureSlots;
	for (size_t i = 0; i < 64; ++i)
	{
		textureSlots[i] = i;
	}

	glUseProgram(m_Shader->GetRendererID());
	int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Textures");
	if (location >= 0)
		glUniform1iv(location, maxTexUnit, &textureSlots[0]);
	glUseProgram(0);
}

void Pipeline::initShaders()
{
	using namespace GLCore::Utils;

	m_Shader = Shader::FromGLSLTextFiles(
		"assets/shaders/test.vert.glsl",
		"assets/shaders/test.frag.glsl"
	);

	m_TextureViewShader = Shader::FromGLSLTextFiles(
		"assets/shaders/texture_viewer.vert.glsl",
		"assets/shaders/texture_viewer.frag.glsl"
	);

	m_ShadowMapShader = Shader::FromGLSLTextFiles(
		"assets/shaders/shadow_map.vert.glsl",
		"assets/shaders/shadow_map.frag.glsl"
	);
}

void Pipeline::initFramebuffers()
{
	using namespace GLCore;

	Window* win = &Application::Get().GetWindow();

	// Fbo
	{
		glCreateTextures(GL_TEXTURE_2D, 2, m_TextureAttachment);
		glBindTexture(GL_TEXTURE_2D, m_TextureAttachment[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, win->GetWidth(), win->GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_GBuffer.norm);
		glBindTexture(GL_TEXTURE_2D, m_GBuffer.norm);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, win->GetWidth(), win->GetHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glBindTexture(GL_TEXTURE_2D, m_TextureAttachment[1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, win->GetWidth(), win->GetHeight(), 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

		glCreateFramebuffers(1, &m_FboID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FboID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureAttachment[0], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBuffer.norm, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_TextureAttachment[1], 0);

		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Fbo not completed." << std::endl;
			GLCORE_ASSERT(false, "");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	// Fbo for shadow maps
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureAttachmentShadowFbo);
		glBindTexture(GL_TEXTURE_2D, m_TextureAttachmentShadowFbo);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		const float borderColor[3] = { 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

		glCreateFramebuffers(1, &m_FboIDShadow);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FboIDShadow);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_TextureAttachmentShadowFbo, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Fbo not completed." << std::endl;
			GLCORE_ASSERT(false, "");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Fbo for g buffers
#if 0
	{
		glCreateFramebuffers(1, &m_FboGBuffersID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FboGBuffersID);

		// create textures
		glCreateTextures(GL_TEXTURE_2D, 1, &m_GBuffer.pos);
		glBindTexture(GL_TEXTURE_2D, m_GBuffer.pos);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, win->GetWidth(), win->GetHeight(), 0, GL_RGB, GL_FLOAT, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GBuffer.pos, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_GBuffer.norm);
		glBindTexture(GL_TEXTURE_2D, m_GBuffer.norm);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, win->GetWidth(), win->GetHeight(), 0, GL_RGB, GL_FLOAT, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_GBuffer.norm, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_GBuffer.diffSpec);
		glBindTexture(GL_TEXTURE_2D, m_GBuffer.diffSpec);
		// TODO: check: format = RGBA, and type = UNSIGNED_BYTE. Sure ??
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, win->GetWidth(), win->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_GBuffer.diffSpec, 0);

		unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);

		// add renderbuffer for depth_attachment
		// ...

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Fbo not completed." << std::endl;
			GLCORE_ASSERT(false, "");
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
#endif

}

void Pipeline::addTexture(const std::string& texturePath)
{
	using namespace GLCore::Utils;
	int nrChannels = -1;
	int width = -1;
	int height = -1;
	unsigned char* pixels = load_image(texturePath.c_str(), &width, &height, &nrChannels, false);
	if (pixels)
	{
		GLenum format = GL_RGB;
		switch (nrChannels)
		{
		case 3: { format = GL_RGB; break; }
		case 4: { format = GL_RGBA; break; }
		default: {GLCORE_ASSERT("nr channels {0} not supported.", nrChannels); }
		}
		GLuint newTextureID;
		glCreateTextures(GL_TEXTURE_2D, 1, &newTextureID);
		glBindTexture(GL_TEXTURE_2D, newTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, (void*)pixels);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_TextureID.push_back(newTextureID);
	}
	else
	{
		GLCORE_ASSERT("Texture {0} could not be loaded.", texturePath);
	}

	free_image(pixels);
}

void Pipeline::initVertexArrays()
{
	// Geometry Triangle
	{
		glCreateVertexArrays(1, &m_TriangleVAO.id);
		glBindVertexArray(m_TriangleVAO.id);

		float vertices[] = {
			/* pos */ -0.5f, -0.5f, 0.0f,  /* norm */ 0.0f, 0.0f, 1.0f,  /* uv */ 0.0f, 0.0f,
			/* pos */  0.5f, -0.5f, 0.0f,  /* norm */ 0.0f, 0.0f, 1.0f,  /* uv */ 1.0f, 0.0f,
			/* pos */  0.0f,  0.5f, 0.0f,  /* norm */ 0.0f, 0.0f, 1.0f,  /* uv */ 0.5f, 1.0f,
		};

		glCreateBuffers(1, &m_TriangleVB);
		glBindBuffer(GL_ARRAY_BUFFER, m_TriangleVB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

		std::vector<uint32_t> indices = { 0, 1, 2 };
		glCreateBuffers(1, &m_TriangleIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_TriangleIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), &indices[0], GL_DYNAMIC_DRAW);
		m_TriangleVAO.numIndices = indices.size();
	}

	// Geometry quad
	{
		glCreateVertexArrays(1, &m_QuadVAO.id);
		glBindVertexArray(m_QuadVAO.id);

		float vertices[] = {
			/* pos */ -0.5f, -0.5f, 0.0f,  /* norm */ 0.0f, 0.0f, 1.0f,  /* uv */ 0.0f, 0.0f,
			/* pos */  0.5f, -0.5f, 0.0f,  /* norm */ 0.0f, 0.0f, 1.0f,  /* uv */ 1.0f, 0.0f,
			/* pos */  0.5f,  0.5f, 0.0f,  /* norm */ 0.0f, 0.0f, 1.0f,  /* uv */ 1.0f, 1.0f,
			/* pos */ -0.5f,  0.5f, 0.0f,  /* norm */ 0.0f, 0.0f, 1.0f,  /* uv */ 0.0f, 1.0f,
		};

		glCreateBuffers(1, &m_QuadVB);
		glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

		std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };
		glCreateBuffers(1, &m_QuadIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), &indices[0], GL_DYNAMIC_DRAW);
		m_QuadVAO.numIndices = indices.size();
	}

	// Geometry circle
	{
		std::vector<float> vertices;
		vertices.push_back(0.0f);
		vertices.push_back(0.0f);
		vertices.push_back(0.0f);
		
		vertices.push_back(0.0f);
		vertices.push_back(0.0f);
		vertices.push_back(1.0f);

		vertices.push_back(0.5f);
		vertices.push_back(0.5f);

		int res = 20;
		float theta = glm::radians(360.0f / res);
		for (size_t i = 0; i < res; ++i)
		{
			glm::vec2 r = 0.5f * glm::vec2{ cos(i * theta), sin(i * theta) };
			vertices.push_back(r.x);
			vertices.push_back(r.y);
			vertices.push_back(0.0f);
			
			vertices.push_back(0.0f);
			vertices.push_back(0.0f);
			vertices.push_back(1.0f);

			vertices.push_back(r.x + 0.5f);
			vertices.push_back(r.y + 0.5f);
		}

		std::vector<uint32_t> indices;
		for (size_t i = 1; i < vertices.size() / 8; i += 1)
		{
			indices.push_back(0);
			indices.push_back(i);
			auto next = i + 1;
			if (next == vertices.size() / 8)
			{
				next = 1;
			}
			indices.push_back(next);
		}

		m_CircleVAO.numIndices = indices.size();
		glCreateVertexArrays(1, &m_CircleVAO.id);
		glBindVertexArray(m_CircleVAO.id);

		glCreateBuffers(1, &m_CircleVB);
		glBindBuffer(GL_ARRAY_BUFFER, m_CircleVB);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

		glCreateBuffers(1, &m_CircleIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_CircleIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_DYNAMIC_DRAW);

		glBindVertexArray(0);
	}
}