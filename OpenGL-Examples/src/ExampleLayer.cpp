#include "ExampleLayer.h"

using namespace GLCore;
using namespace GLCore::Utils;

extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

ExampleLayer::ExampleLayer()
	: m_CameraController(16.0f / 9.0f)
{

}

ExampleLayer::~ExampleLayer()
{

}

void ExampleLayer::OnAttach()
{
	EnableGLDebugging();

#if 0
	std::vector<std::string> texturePaths = {
	 "assets/textures/0.png",
	 "assets/textures/1.png",
	 "assets/textures/001_4.png",
	 "assets/textures/2.png",
	 "assets/textures/002_4.png",
	 "assets/textures/3.png",
	 "assets/textures/003_2.png",
	 "assets/textures/4.png",
	 "assets/textures/004_1.png",
	 "assets/textures/5.png",
	 "assets/textures/005_4.png",
	 "assets/textures/6.png",
	 "assets/textures/006_0.png",
	 "assets/textures/7.png",
	 "assets/textures/8.png",
	 "assets/textures/9.png",
	 "assets/textures/009_0.png",
	 "assets/textures/10.png",
	 "assets/textures/11.png",
	 "assets/textures/ReflectingTheLava.png",
	 "assets/textures/chess.png",
	 "assets/textures/chesterfieldsofa_basecolor.png",
	 "assets/textures/diffuse_brickwall.jpg",
	 "assets/textures/diffuse_cube.png",
	 "assets/textures/diffuse_marble.png",
	 "assets/textures/lava.png",
	 "assets/textures/lava final.png",
	 "assets/textures/lol.jpg",
	 "assets/textures/normal_brickwall.jpg",
	 "assets/textures/normal_cube.png",
	 "assets/textures/normal_marble.png",
	 "assets/textures/specular_brickwall.jpg",
	 "assets/textures/specular_cube.png",
	 "assets/textures/specular_marble.png",
	 "assets/textures/text7.jpg",
	 "assets/textures/tile01.png",
	 "assets/textures/tile02.png",
	 "assets/textures/tile03.png",
	 "assets/textures/tile05.png",
	 "assets/textures/tile06.png",
	 "assets/textures/tile07.png",
	 "assets/textures/tile07.png"
	};

#else
	std::vector<std::string> texturePaths = {
		"assets/textures/diffuse_brickwall.jpg",
		"assets/textures/normal_brickwall.jpg",
		"assets/textures/specular_brickwall.jpg",

		"assets/textures/diffuse_cube.png",
		"assets/textures/normal_cube.png",
		"assets/textures/specular_cube.png",

		"assets/textures/diffuse_marble.png",
		"assets/textures/normal_marble.png",
		"assets/textures/specular_marble.png",

		"assets/textures/diffuse_brickwall.jpg",
		"assets/textures/normal_brickwall.jpg",
		"assets/textures/specular_brickwall.jpg",

		"assets/textures/diffuse_cube.png",
		"assets/textures/normal_cube.png",
		"assets/textures/specular_cube.png",

		"assets/textures/diffuse_marble.png",
		"assets/textures/normal_marble.png",
		"assets/textures/specular_marble.png",

		"assets/textures/diffuse_brickwall.jpg",
		"assets/textures/normal_brickwall.jpg",
		"assets/textures/specular_brickwall.jpg",

		"assets/textures/diffuse_cube.png",
		"assets/textures/normal_cube.png",
		"assets/textures/specular_cube.png",

		"assets/textures/diffuse_marble.png",
		"assets/textures/normal_marble.png",
		"assets/textures/specular_marble.png",

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
#endif

	for (auto& s : texturePaths)
	{
		m_Pipeline.addTexture(s);
	}

	m_Pipeline.init();

#ifdef GLCORE_DEBUG
	m_InstanceData.resize(100);
#else
	m_InstanceData.resize(50000);
#endif
	for (auto i = 0; i < m_InstanceData.size(); ++i)
	{
		glm::vec3 displace;
		float invScale;
		displace.x = 16.0 * 0.33f * (-0.25f + 0.5f * rand() / (float(RAND_MAX) + 1));
		displace.y = 9.0 *  0.33f * (-0.25f + 0.5f * rand() / (float(RAND_MAX) + 1));
		displace.z = 0.5 * rand() / (float(RAND_MAX) + 1);
#ifdef GLCORE_DEBUG
		invScale = (rand() / (float(RAND_MAX) + 1)) < 0.5f ? 1.0 : 2.0f;
#else
		invScale = (rand() / (float(RAND_MAX) + 1)) < 0.5f ? 50.0 : 100.0f;
#endif
		if (i == 0)
		{
			invScale /= 2.0;
			displace.x = 0.0f;
			displace.y = 0.0f;
			displace.z = 1.0f;
		} 

		m_InstanceData[i].model = glm::scale(glm::translate(glm::mat4{ 1.0 }, displace), glm::vec3{ 1.0f / invScale });
		m_InstanceData[i].color.x = rand() / (float(RAND_MAX) + 1);
		m_InstanceData[i].color.y = rand() / (float(RAND_MAX) + 1);
		m_InstanceData[i].color.z = rand() / (float(RAND_MAX) + 1);
		m_InstanceData[i].color.w = 1.0f;

		auto tex = 3 * int((rand() / (float(RAND_MAX) + 1)) * floor(texturePaths.size() / 3.0f)  );
		//auto tex = int((rand() / (float(RAND_MAX) + 1)) * m_TexturePaths.size());
		
		m_InstanceData[i].texUnitDiff = tex + 0;
		m_InstanceData[i].texUnitSpec = tex + 1;
		m_InstanceData[i].texUnitNorm = tex + 2;
		m_InstanceData[i].specularity = 128;
		
		if (i % 3 == 0)
		{
			m_InstanceData[i].vao = m_Pipeline.getCircleVAO();
		}
		else if (i % 3 == 1)
		{
			m_InstanceData[i].vao = m_Pipeline.getQuadVAO();
		}
		else if (i % 3 == 2)
		{
			m_InstanceData[i].vao = m_Pipeline.getTriangleVAO();
		}

	}
}

void ExampleLayer::OnDetach()
{
}

void ExampleLayer::OnEvent(Event& event)
{
	m_CameraController.OnEvent(event);

	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<MouseButtonPressedEvent>(
		[&](MouseButtonPressedEvent& e)
		{
			//m_SquareColor = m_SquareAlternateColor;
			return false;
		});
	dispatcher.Dispatch<MouseButtonReleasedEvent>(
		[&](MouseButtonReleasedEvent& e)
		{
			//m_SquareColor = m_SquareBaseColor;
			return false;
		});
	dispatcher.Dispatch<WindowResizeEvent>(
		[&](WindowResizeEvent& e)
		{
			//SetFramebuffers(); // TODO: do it 1 sec after the last resize.
			return false;
		});
}

void ExampleLayer::OnUpdate(Timestep ts)
{
	auto start = std::chrono::high_resolution_clock::now();

	auto timeInMicsec = std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count();
	static int frameCount = 0;
	for (auto& instance : m_InstanceData)
	{
		// simulate dynamic changing of instance data
		//if (frameCount % 480 == 479)
		//{
		//	auto tex = 3 * int((rand() / (float(RAND_MAX) + 1)) * floor(m_Pipeline.getTextures().size() / 3.0f));
		//	instance.texUnitDiff = 0 + 0;
		//	instance.texUnitSpec = 0 + 1;
		//	instance.texUnitNorm = 0 + 2;
		//}
		float dx = 0.005 * (-0.5f + rand() / (float(RAND_MAX) + 1));
		float dy = 0.005 * (-0.5f + rand() / (float(RAND_MAX) + 1));
		instance.model = glm::translate(instance.model, glm::vec3{dx, dy, 0.0f});
	}
	++frameCount;

	m_CameraController.OnUpdate(ts);

	m_Pipeline.setCamera(&m_CameraController.GetCamera());
	m_Pipeline.setObjects(&m_InstanceData);
	m_Pipeline.update();

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = end - start;
	m_AvgDuration = duration / std::chrono::microseconds(1);
}

float values_getter(void* data, int idx)
{
	return (float)(*((float*)data + idx));
}

void ExampleLayer::OnImGuiRender()
{

	m_PipelineImGuiController.update();

	ImGui::Begin("Info");

	bool vsync = Application::Get().GetWindow().IsVSync();
	if (ImGui::Button("VSync"))
	{
		vsync = !vsync;
		Application::Get().GetWindow().SetVSync(vsync);
	}
	ImGui::SameLine();
	ImGui::Text(vsync ? "On" : "Off");
	ImGui::Text("FPS: %.1f  (%.1f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("Draw calls: %d", m_Pipeline.getRenderer().getNumDrawCalls());
	ImGui::Text("Avg FlushBatch size: %f", m_Pipeline.getRenderer().getAvgFlushBatchSize());

	if(ImGui::Button("ImGui Demo"))
		ImGui::ShowDemoWindow();

	float fps = ImGui::GetIO().Framerate;
	float frameTime = 1000.0f / fps;

	static float values[180] = { 0 };
	static int values_offset = 0;
	static double refresh_time = 0.0;

	if (refresh_time == 0.0)
		refresh_time = ImGui::GetTime();
	while (refresh_time < ImGui::GetTime())
	{
		static float phase = 0.0f;
		values[values_offset] = m_AvgDuration;
		values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
		phase += 0.10f*values_offset;
		refresh_time += 1.0f / 60.0f;
	}
	{
		float average = 0.0f;
		for (int n = 0; n < IM_ARRAYSIZE(values); n++)
			average += values[n];
		average /= (float)IM_ARRAYSIZE(values);
		char overlay[128];
		sprintf(overlay, "avg time: %f microsec", average);
		ImGui::PlotLines("", values, IM_ARRAYSIZE(values), values_offset, overlay, 0.0, 2.0 * average, ImVec2(0, 160));
	}

	ImGui::End();
	
}
