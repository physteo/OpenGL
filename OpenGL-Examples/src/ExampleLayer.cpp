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

	m_TexturePaths = {
		"assets/textures/diffuse_brickwall.jpg",
		"assets/textures/normal_brickwall.jpg",
		"assets/textures/specular_brickwall.jpg",
		"assets/textures/diffuse_cube.png",
		"assets/textures/normal_cube.png",
		"assets/textures/specular_cube.png",
		"assets/textures/diffuse_marble.png",
		"assets/textures/normal_marble.png",
		"assets/textures/specular_marble.png",
		"assets/textures/specular_marble.png",
		"assets/textures/specular_marble.png",
		"assets/textures/specular_marble.png",
		"assets/textures/specular_marble.png",
		"assets/textures/specular_marble.png",
		"assets/textures/specular_marble.png",
		"assets/textures/specular_marble.png"
	};

	// Textures
	LoadTextures();
	m_StreamMode = BUFFER;
}

ExampleLayer::~ExampleLayer()
{

}

void ExampleLayer::LoadGeometry()
{

	// Geometry quad
	{
		glCreateVertexArrays(1, &m_QuadVA);
		glBindVertexArray(m_QuadVA);

		float vertices[] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f,	1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f,	1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f,	0.0f, 1.0f,
		};

		glCreateBuffers(1, &m_QuadVB);
		glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(0));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

		uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
		glCreateBuffers(1, &m_QuadIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
	}

	// Geometry circle
	{
		float vertices[] =
		{
			-0.75f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.75f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f,	1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f,	0.0f, 1.0f,
		};
		uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

		glCreateVertexArrays(1, &m_CircleVA);
		glBindVertexArray(m_CircleVA);

		glCreateBuffers(1, &m_CircleVB);
		glBindBuffer(GL_ARRAY_BUFFER, m_CircleVB);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

		glCreateBuffers(1, &m_CircleIB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_CircleIB);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);

		glBindVertexArray(0);
	}

}

void ExampleLayer::LoadTextures()
{
	m_TextureID.resize(m_TexturePaths.size());
	glCreateTextures(GL_TEXTURE_2D, m_TexturePaths.size(), &m_TextureID[0]);

	for (auto i = 0; i < m_TexturePaths.size(); ++i)
	{
		
		int nrChannels = -1;
		int width = -1;
		int height = -1;
		unsigned char* pixels = GLCore::Utils::load_image(m_TexturePaths[i].c_str(), &width, &height, &nrChannels, false);
		if (pixels)
		{
			GLenum format = GL_RGB;
			switch (nrChannels)
			{
				case 3: { format = GL_RGB; break; }
				case 4: { format = GL_RGBA; break; }
				default: {GLCORE_ASSERT("nr channels {0} not supported.", nrChannels); }
			}

			glBindTexture(GL_TEXTURE_2D, m_TextureID[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, (void*)pixels);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			GLCORE_ASSERT("Texture {0} could not be loaded.", m_TexturePaths[i]);
		}

		GLCore::Utils::free_image(pixels);
	}
}

void ExampleLayer::OnAttach()
{
	EnableGLDebugging();

	// OpenGL states
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Shaders
	m_Shader = Shader::FromGLSLTextFiles(
		"assets/shaders/test.vert.glsl",
		"assets/shaders/test.frag.glsl"
	);

	LoadTextures();

	LoadGeometry();

	m_InstanceData.resize(100000);
	for (auto i = 0; i < m_InstanceData.size(); ++i)
	{
		m_InstanceData[i].displace.x = 16.0 * 0.33f * (-0.25f + 0.5f * rand() / (float(RAND_MAX) + 1));
		m_InstanceData[i].displace.y = 9.0 *  0.33f * (-0.25f + 0.5f * rand() / (float(RAND_MAX) + 1));
		m_InstanceData[i].displace.z = 0.0f;
		m_InstanceData[i].displace.w = (rand() / (float(RAND_MAX) + 1)) < 0.5f ? 100.0 : 200.0f;

		m_InstanceData[i].color.x = rand() / (float(RAND_MAX) + 1);
		m_InstanceData[i].color.y = rand() / (float(RAND_MAX) + 1);
		m_InstanceData[i].color.z = rand() / (float(RAND_MAX) + 1);
		m_InstanceData[i].color.w = 1.0f;

		auto tex = int((rand() / (float(RAND_MAX) + 1)) * m_TexturePaths.size());
		m_InstanceData[i].texUnitDiff = (tex + 0) % m_TexturePaths.size();//int((rand() / (float(RAND_MAX) + 1)) * m_TexturePaths.size());
		m_InstanceData[i].texUnitSpec = (tex + 1) % m_TexturePaths.size();//int((rand() / (float(RAND_MAX) + 1)) * m_TexturePaths.size());
		m_InstanceData[i].texUnitNorm = (tex + 2) % m_TexturePaths.size();//int((rand() / (float(RAND_MAX) + 1)) * m_TexturePaths.size());
		m_InstanceData[i].vaoID = (i % 2 == 0) ? m_CircleVA : m_QuadVA;
	}

	renderer.setShader(m_Shader);
	renderer.setTextures(&m_TextureID);
	renderer.init();

}

void ExampleLayer::OnDetach()
{
	glDeleteVertexArrays(1, &m_QuadVA);
	glDeleteBuffers(1, &m_QuadVB);
	glDeleteBuffers(1, &m_QuadIB);
}

void ExampleLayer::OnEvent(Event& event)
{
	m_CameraController.OnEvent(event);

	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<MouseButtonPressedEvent>(
		[&](MouseButtonPressedEvent& e)
		{
			m_SquareColor = m_SquareAlternateColor;
			return false;
		});
	dispatcher.Dispatch<MouseButtonReleasedEvent>(
		[&](MouseButtonReleasedEvent& e)
		{
			m_SquareColor = m_SquareBaseColor;
			return false;
		});
}

void ExampleLayer::OnUpdate(Timestep ts)
{
	auto start = std::chrono::high_resolution_clock::now();

	m_CameraController.OnUpdate(ts);

	glUseProgram(m_Shader->GetRendererID());

	{
		int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewProjection");
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));
	}

	renderer.begin();
	for (auto instance : m_InstanceData)
	{
		VertexArray mesh;
		mesh.setID(instance.vaoID);
		Transform tr;
		tr.position = instance.displace;
		Material mat;
		mat.diffuse = instance.texUnitDiff;
		mat.specular = instance.texUnitSpec;
		mat.normal = instance.texUnitNorm;
		renderer.submit(mesh, tr, mat);
	}

	renderer.flush();
	renderer.end();
	//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glBindBuffer(GL_ARRAY_BUFFER, m_InstancesVB);
	//
	//switch (m_StreamMode) {
	//	case BUFFER:
	//	{
	//		glBufferData(GL_ARRAY_BUFFER, m_InstanceData.size() * sizeof(InstanceData), (void*)&(m_InstanceData[0]), GL_DYNAMIC_DRAW);
	//		break;
	//	}
	//	case SUB:
	//	{
	//		glBufferSubData(GL_ARRAY_BUFFER, 0, m_InstanceData.size() * sizeof(InstanceData), (void*)&(m_InstanceData[0]));
	//		break;
	//	}
	//	case NONE: ;
	//	{
	//		break;
	//	}
	//	case MAP:
	//	{
	//		void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	//		memcpy(ptr, &(m_InstanceData[0]), m_InstanceData.size() * sizeof(InstanceData));
	//		glUnmapBuffer(GL_ARRAY_BUFFER);
	//		break;              
	//	};
	//}
	//
	//glBindVertexArray(m_QuadVA);
	//glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, m_InstanceData.size());
	//glBindVertexArray(0);

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


	ImGui::Begin("Controls");
	
	ImGui::Text("Draw calls: %d", renderer.getNumDrawCalls());
	ImGui::Text("Avg FlushBatch size: %f", renderer.getAvgFlushBatchSize());

	static int maxTexUnit = 32;
	ImGui::SliderInt("Max Texture Units", &maxTexUnit, 1, 64);
	renderer.setMaxTextureUnits(maxTexUnit);

	//if(ImGui::Button("ImGui Demo"))
		ImGui::ShowDemoWindow();

	static StreamMode selected = BUFFER;
	if (ImGui::Selectable("glBufferData", selected == BUFFER))
		selected = BUFFER;
	if (ImGui::Selectable("glBufferSubData", selected == SUB))
		selected = SUB;
	if (ImGui::Selectable("glMap/glUnmap", selected == MAP))
		selected = MAP;
	if (ImGui::Selectable("Static(no stream)", selected == NONE))
		selected = NONE;
	m_StreamMode = selected;

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
