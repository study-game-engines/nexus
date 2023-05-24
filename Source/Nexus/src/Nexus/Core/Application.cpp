#include "nxpch.h"
#include "GLFW/glfw3.h"

#include "Application.h"
#include "Input.h"
#include "FileDialog.h"

#include "Renderer/Context.h"
#include "Renderer/Renderer.h"

#include "Assets/AssetManager.h"

#include "Script/ScriptEngine.h"

#include "Physics/PhysicsEngine.h"

Nexus::Application* Nexus::Application::s_Instance = nullptr;

static std::vector<Nexus::Layer*> m_layerStack;

Nexus::Application::Application()
{
	s_Instance = this;

	NEXUS_LOG_INIT

	m_AppSpecs = { 800,600,"Nexus",false,false,RenderAPI_Vulkan };
	m_Window = {};

	if (!glfwInit())
	{
		NEXUS_ASSERT(1, "glfw Initialization Failed");
	}

	NEXUS_LOG_TRACE("Nexus::Core Initialized");
}

Nexus::Application::~Application()
{
	glfwTerminate();
	NEXUS_LOG_TRACE("Nexus::Core Terminated");
	NEXUS_LOG_SHUT
}

void Nexus::Application::Init()
{
	// Window Creation
	{
		m_Window.width = m_AppSpecs.Window_Width;
		m_Window.height = m_AppSpecs.Window_height;
		m_Window.title = m_AppSpecs.Window_Title;
		m_Window.handle = nullptr;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

		m_Window.handle = glfwCreateWindow(m_Window.width, m_Window.height, m_Window.title, nullptr, nullptr);
		NEXUS_LOG_TRACE("{2} Window Created: {0}x{1}", m_Window.width, m_Window.height, m_Window.title);

		// Callbacks
		{
			glfwSetWindowUserPointer(m_Window.handle, &m_Window);

			glfwSetWindowSizeCallback(m_Window.handle, [](GLFWwindow* window, int width, int height)
				{
					Window& data = *(Window*)glfwGetWindowUserPointer(window);
					data.width = width;
					data.height = height;
				});
		}

		Input::SetContextWindow(m_Window);
		FileDialog::SetContextWindow(m_Window);
	}
	
	std::cout << std::endl;

	if(m_AppSpecs.rApi != RenderAPI_None)
	{
		RendererSpecifications specs{};
		specs.vsync = m_AppSpecs.Vsync;
		specs.msaa = m_AppSpecs.MultiSampling;

		if (m_AppSpecs.rApi == RenderAPI_Vulkan)
			specs.api = RenderAPIType::VULKAN;
		
		Renderer::Init(specs);
		Renderer::ResizeCallback = NEXUS_BIND_FN(Application::ResizeCallback,this);

		AssetManager::Initialize(m_AppSpecs.LoadDefaultAssets);
	}

	if (m_AppSpecs.EnableScriptEngine)
		ScriptEngine::Init();

	if (m_AppSpecs.pApi != PhysicsAPI_None)
	{
		PhysicsAPIType pApi = PhysicsAPIType::None;
		if (m_AppSpecs.pApi == PhysicsAPI_Jolt)
			pApi = PhysicsAPIType::Jolt;

		PhysicsEngine::Initialize(pApi);
	}
}


void Nexus::Application::Run()
{
	std::cout << std::endl;

	{
		for (auto& l : m_layerStack)
			l->OnAttach();
	
		if (m_AppSpecs.rApi != RenderAPI_None)
			Renderer::FlushTransferCommandQueue();
	}

	glfwShowWindow(m_Window.handle);
	while (!glfwWindowShouldClose(m_Window.handle))
	{
		NEXUS_SCOPED_PROFILE("Application Loop");

		glfwPollEvents();

		// TimeStep
		{
			static float ct, lt;
			
			ct = (float)glfwGetTime();
			m_TimeStep = Timestep(ct - lt);
			lt = ct;
		}

		// Update
		{
			NEXUS_SCOPED_PROFILE("On Update");

			for (auto& l : m_layerStack)
			{
				l->OnUpdate(m_TimeStep);
			}

		}

		// Rendering
		if (m_AppSpecs.rApi != RenderAPI_None)
		{
			Renderer::FlushTransferCommandQueue();

			NEXUS_SCOPED_PROFILE("On Render");

			Renderer::BeginRenderCommandQueue();

			for (auto& l : m_layerStack)
				l->OnRender();

			Renderer::EndRenderCommandQueue();
			Renderer::FlushRenderCommandQueue();
		}
	}
	
	if (m_AppSpecs.rApi != RenderAPI_None)
		Renderer::WaitForDevice();

	for (auto& l : m_layerStack)
	{
		l->OnDetach();
		PopLayer(l);
		delete l;
	}
	std::cout << std::endl;
}

void Nexus::Application::Shut()
{
	if (m_AppSpecs.pApi != PhysicsAPI_None)
		PhysicsEngine::Shutdown();

	if (m_AppSpecs.EnableScriptEngine)
		ScriptEngine::Shut();

	if (m_AppSpecs.rApi != RenderAPI_None)
	{
		AssetManager::Shutdown();
		Renderer::Shut();
	}
	
	// Window Destruction
	{
		glfwDestroyWindow(m_Window.handle);
		NEXUS_LOG_TRACE("Window Destroyed");
	}
}

void Nexus::Application::SetWindowTitle(const char* name)
{
	glfwSetWindowTitle(m_Window.handle, name);
}

void Nexus::Application::ResizeCallback()
{
	for (auto& layer : m_layerStack)
	{
		layer->OnWindowResize(m_Window.width, m_Window.height);
	}
}

void Nexus::Application::PushLayer(Layer* layer)
{
	m_layerStack.push_back(layer);
}

void Nexus::Application::PopLayer(Layer* layer)
{
	auto it = std::find(m_layerStack.begin(), m_layerStack.end(), layer);
	if (it != m_layerStack.end())
	{
		m_layerStack.erase(it);
	}
}
