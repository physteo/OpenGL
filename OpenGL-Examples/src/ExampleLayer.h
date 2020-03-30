#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

#include "Renderer.h"

struct InstanceData
{
	unsigned int vaoID;
	glm::vec4 displace;
	glm::vec4 color;
	float texUnit;
};

enum StreamMode
{
	NONE,
	BUFFER,
	SUB,
	MAP
};

class ExampleLayer : public GLCore::Layer
{
public:
	ExampleLayer();
	virtual ~ExampleLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;
private:
	void LoadTextures();
	void LoadGeometry();

	GLCore::Utils::Shader* m_Shader;
	GLCore::Utils::OrthographicCameraController m_CameraController;
	
	StreamMode m_StreamMode;
	std::vector<InstanceData> m_InstanceData;
	float m_AvgDuration = 0.0f;

	GLuint m_QuadVA, m_QuadVB, m_QuadIB;
	GLuint m_CircleVA, m_CircleVB, m_CircleIB;
	GLuint m_InstancesVB;

	std::vector<std::string> m_TexturePaths = {
		"assets/textures/diffuse_brickwall.jpg",
		"assets/textures/normal_brickwall.jpg",
		"assets/textures/specular_brickwall.jpg",
		"assets/textures/diffuse_cube.png",
		"assets/textures/normal_cube.png",
		"assets/textures/specular_cube.png",
		"assets/textures/diffuse_marble.png",
		"assets/textures/normal_marble.png",
		"assets/textures/specular_marble.png"
	};
	std::vector<GLuint> m_TextureID;

	glm::vec4 m_SquareBaseColor = { 0.8f, 0.2f, 0.3f, 1.0f };
	glm::vec4 m_SquareAlternateColor = { 0.2f, 0.3f, 0.8f, 1.0f };
	glm::vec4 m_SquareColor = m_SquareBaseColor;

	Renderer renderer;
};