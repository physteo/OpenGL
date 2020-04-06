#pragma once

#include "BatchRenderer.h"
#include "FlushRenderer.h"

struct InstanceData
{
	VertexArray vao;

	glm::mat4 model;
	glm::vec4 color;
	float texUnitDiff;
	float texUnitSpec;
	float texUnitNorm;
	float specularity;
};

class PipelineImGuiController;

class Pipeline
{
	friend class PipelineImGuiController;

public:
	~Pipeline();

	void init();
	void update();

	const BatchRenderer& getRenderer() const { return m_batchRenderer; }
	void setMaxTextureUnits(unsigned int maxTexUnit);
	inline unsigned int getMaxTextureUnits() const { return m_batchRenderer.getMaxTextureUnits(); }

	void addTexture(const std::string& texturePath);
	inline void setCamera(GLCore::Utils::OrthographicCamera* camera) { m_camera = camera; }
	inline void setObjects(std::vector<InstanceData>* objects) { m_objects = objects; }
	inline VertexArray getQuadVAO() const { return m_QuadVAO; };
	inline VertexArray getCircleVAO() const { return m_CircleVAO; };
	inline VertexArray getTriangleVAO() const { return m_TriangleVAO; };
private:
	void initShaders();
	void initFramebuffers();
	void initVertexArrays();

	// Data to draw
	std::vector<InstanceData>* m_objects;
	
	// Lights
	glm::vec3 m_sunDir{0.0f, -0.13f, -1.0f};

	// Camera
	GLCore::Utils::OrthographicCamera* m_camera;

	// Renderer
	BatchRenderer m_batchRenderer;

	// FBOs
	GLuint m_FboID;
	GLuint m_TextureAttachment[2];
	GLuint m_FboIDShadow;
	GLuint m_TextureAttachmentShadowFbo;
	GLuint m_FboGBuffersID;
	struct GBuffer
	{
		GLuint pos;
		GLuint norm;
		GLuint diffSpec;
	} m_GBuffer;
	
	// Assets
	// ... geometry
	VertexArray m_QuadVAO;
	VertexArray m_CircleVAO;
	VertexArray m_TriangleVAO;
	GLuint m_QuadVB, m_QuadIB;
	GLuint m_CircleVB, m_CircleIB;
	GLuint m_TriangleVB, m_TriangleIB;
	// ... textures
	std::vector<GLuint> m_TextureID;
	// ... shaders
	GLCore::Utils::Shader* m_Shader;
	GLCore::Utils::Shader* m_TextureViewShader;
	GLCore::Utils::Shader* m_ShadowMapShader;

	// Output texture
	int m_attachment{ 0 };
};

class PipelineImGuiController
{
public:
	PipelineImGuiController(Pipeline* pipeline) : m_pipeline(pipeline) {}

	void update();

private:
	Pipeline* m_pipeline;
};