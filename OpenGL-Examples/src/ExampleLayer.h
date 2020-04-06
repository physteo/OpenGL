#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>

#include "Pipeline.h"

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
	Pipeline m_Pipeline;
	PipelineImGuiController m_PipelineImGuiController{ &m_Pipeline };

	GLCore::Utils::OrthographicCameraController m_CameraController;
	std::vector<InstanceData> m_InstanceData;

	float m_AvgDuration = 0.0f;
};