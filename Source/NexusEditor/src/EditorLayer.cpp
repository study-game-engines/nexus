#include "EditorLayer.h"

void EditorLayer::OnAttach()
{
	NEXUS_LOG_DEBUG("Editor Layer Attached");

	// Graphics Renderpass
	{
		std::vector<Nexus::RenderpassAttachmentDescription> attachments;
		{
			auto& color = attachments.emplace_back();
			color.type = Nexus::ImageType::Color;
			color.multiSampled = true;
			color.load = Nexus::ImageOperation::Clear;
			color.store = Nexus::ImageOperation::Store;
			color.initialLayout = Nexus::ImageLayout::Undefined;
			color.finalLayout = Nexus::ImageLayout::ColorAttachment;

			auto& depth = attachments.emplace_back();
			depth.type = Nexus::ImageType::Depth;
			depth.multiSampled = true;
			depth.load = Nexus::ImageOperation::Clear;
			depth.store = Nexus::ImageOperation::DontCare;
			depth.initialLayout = Nexus::ImageLayout::Undefined;
			depth.finalLayout = Nexus::ImageLayout::DepthStencilAttachment;

			auto& resolve = attachments.emplace_back();
			resolve.type = Nexus::ImageType::Resolve;
			resolve.multiSampled = false;
			resolve.load = Nexus::ImageOperation::DontCare;
			resolve.store = Nexus::ImageOperation::Store;
			resolve.initialLayout = Nexus::ImageLayout::Undefined;
			resolve.finalLayout = Nexus::ImageLayout::ShaderReadOnly;
		}

		std::vector<Nexus::SubpassDescription> subpasses;
		{
			auto& subpass0 = subpasses.emplace_back();
			subpass0.ColorAttachments = { 0 };
			subpass0.DepthAttachment = 1;
			subpass0.ResolveAttachment = 2;
		}

		std::vector<Nexus::SubpassDependency> subpassDependencies;
		{
			auto& dep = subpassDependencies.emplace_back();
			dep.srcSubpass = Nexus::SubpassDependency::ExternalSubpass;
			dep.dstSubpass = 0;
			dep.srcStageFlags = Nexus::PipelineStageFlag::ColorAttachmentOutput;
			dep.dstStageFlags = Nexus::PipelineStageFlag::ColorAttachmentOutput;
			dep.srcAccessFlags = Nexus::AccessFlag::None;
			dep.dstAccessFlags = Nexus::AccessFlag::ColorAttachmentWrite;
		}

		Nexus::RenderpassSpecification specs{};
		specs.attachments = &attachments;
		specs.subpasses = &subpasses;
		specs.dependencies = &subpassDependencies;

		m_GraphicsPass = Nexus::Renderpass::Create(specs);
	}

	// Imgui Renderpass
	{
		std::vector<Nexus::RenderpassAttachmentDescription> attachments;
		{
			auto& color = attachments.emplace_back();
			color.type = Nexus::ImageType::Color;
			color.multiSampled = false;
			color.load = Nexus::ImageOperation::Clear;
			color.store = Nexus::ImageOperation::Store;
			color.initialLayout = Nexus::ImageLayout::Undefined;
			color.finalLayout = Nexus::ImageLayout::PresentSrc;
		}

		std::vector<Nexus::SubpassDescription> subpasses;
		{
			auto& subpass0 = subpasses.emplace_back();
			subpass0.ColorAttachments = { 0 };
		}

		std::vector<Nexus::SubpassDependency> subpassDependencies;
		{
			auto& dep = subpassDependencies.emplace_back();
			dep.srcSubpass = Nexus::SubpassDependency::ExternalSubpass;
			dep.dstSubpass = 0;
			dep.srcStageFlags = Nexus::PipelineStageFlag::ColorAttachmentOutput;
			dep.dstStageFlags = Nexus::PipelineStageFlag::ColorAttachmentOutput;
			dep.srcAccessFlags = Nexus::AccessFlag::None;
			dep.dstAccessFlags = Nexus::AccessFlag::ColorAttachmentWrite;
		}

		Nexus::RenderpassSpecification specs{};
		specs.attachments = &attachments;
		specs.subpasses = &subpasses;
		specs.dependencies = &subpassDependencies;

		m_ImGuiPass = Nexus::Renderpass::Create(specs);
	}

	// Graphics Framebuffer
	{
		auto extent = Nexus::Renderer::GetSwapchain()->GetExtent();

		auto& a1 = m_GraphicsFBspecs.attachments.emplace_back();
		a1.Type = Nexus::FramebufferAttachmentType::Color;
		a1.multisampled = true;
		
		auto& a2 = m_GraphicsFBspecs.attachments.emplace_back();
		a2.Type = Nexus::FramebufferAttachmentType::DepthStencil;
		a2.multisampled = true;
		
		auto& a3 = m_GraphicsFBspecs.attachments.emplace_back();
		a3.Type = Nexus::FramebufferAttachmentType::ShaderReadOnly_Color;
		a3.multisampled = false;
		
		m_GraphicsFBspecs.extent = extent;
		m_GraphicsFBspecs.renderpass = m_GraphicsPass;

		m_GraphicsFramebuffer = Nexus::Framebuffer::Create(m_GraphicsFBspecs);
	}
	
	// ImGui Framebuffer
	{
		auto extent = Nexus::Renderer::GetSwapchain()->GetExtent();

		auto& a1 = m_ImGuiFBspecs.attachments.emplace_back();
		a1.Type = Nexus::FramebufferAttachmentType::PresentSrc;
		a1.multisampled = false;
		
		m_ImGuiFBspecs.extent = extent;
		m_ImGuiFBspecs.renderpass = m_ImGuiPass;

    m_ImGuiFramebuffer = Nexus::Framebuffer::Create(m_ImGuiFBspecs);
	}

	{
		Nexus::EditorContext::Initialize(m_ImGuiPass);

		m_ImGuiEditorViewport = Nexus::EditorViewport::Create();
		m_ImGuiEditorViewport->SetContext(m_GraphicsFramebuffer, 2);
	}

	// Screen
	{
		Nexus::Extent Extent = Nexus::Renderer::GetSwapchain()->GetExtent();

		m_viewport.x = 0.0f;
		m_viewport.y = 0.0f;
		m_viewport.width = (float)Extent.width;
		m_viewport.height = (float)Extent.height;
		m_viewport.minDepth = 0.0f;
		m_viewport.maxDepth = 1.0f;

		m_scissor.Offset = { 0,0 };
		m_scissor.Extent = { Extent.width, Extent.height };
	}

	Nexus::Ref<Nexus::Shader> simpleShader = Nexus::ShaderLib::Get("shaders/simple.shader");
	
	// Pipeline
	{
		Nexus::PipelineCreateInfo Info{};
		Info.shader = simpleShader;
		Info.subpass = 0;
		Info.renderpass = m_GraphicsPass;
		Info.multisampled = true;

		Info.vertexBindInfo = Nexus::StaticMeshVertex::GetBindings();
		Info.vertexAttribInfo = Nexus::StaticMeshVertex::GetAttributes();

		Info.pushConstantInfo.resize(1);
		Info.pushConstantInfo[0].offset = 0;
		Info.pushConstantInfo[0].size = sizeof(glm::mat4);
		Info.pushConstantInfo[0].stage = Nexus::ShaderStage::Vertex;

		Info.rasterizerInfo.lineWidth = 1.f;
		Info.rasterizerInfo.frontFace = Nexus::FrontFaceType::Clockwise;
		Info.rasterizerInfo.cullMode = Nexus::CullMode::Back;
		Info.rasterizerInfo.polygonMode = Nexus::PolygonMode::Fill;

		m_Pipeline = Nexus::Pipeline::Create(Info);
	}

	
	// Camera
	{
		using namespace Nexus;

		m_cameraController.AttachCamera(&m_camera);
		
		m_cameraController.SetKeyBindings(CameraBindings::FRONT, Key::W);
		m_cameraController.SetKeyBindings(CameraBindings::BACK, Key::S);
		m_cameraController.SetKeyBindings(CameraBindings::DOWN, Key::E);
		m_cameraController.SetKeyBindings(CameraBindings::UP, Key::Q);
		m_cameraController.SetKeyBindings(CameraBindings::LEFT, Key::A);
		m_cameraController.SetKeyBindings(CameraBindings::RIGHT, Key::D);

		Nexus::Extent extent = Renderer::GetSwapchain()->GetExtent();
		
		m_cameraController.SetPerspectiveProjection(45.f, (float)extent.width,(float)extent.height, 0.1f, 1000.f);
	}

	// Scene
	{
		Nexus::AssetHandle handle = Nexus::AssetManager::LoadFromFile<Nexus::StaticMeshAsset>("res/Meshes/Suzane.fbx");

		m_Scene = Nexus::Scene::Create();
		m_SceneData = Nexus::SceneBuildData::Build(m_Scene, simpleShader);

		Nexus::Entity entity = m_Scene->CreateEntity();
		entity.AddComponent<Nexus::Component::Mesh>(handle);
		entity.AddComponent<Nexus::Component::Script>("Sandbox.Player");

		m_SceneRenderer.SetContext(m_Scene, m_SceneData);
		m_SceneHeirarchy.SetContext(m_Scene);
	}
}

void EditorLayer::OnUpdate(Nexus::Timestep ts)
{
	glm::vec2 size = m_ImGuiEditorViewport->GetViewportSize();
	if (size != m_ImGuiEditorViewportSize)
	{
		m_ImGuiEditorViewportSize = size;
		m_cameraController.SetPerspectiveProjection(45.f, size.x, size.y, 0.1f, 1000.f);
	}

	m_cameraController.Move();
	m_SceneData->Update(m_Scene, m_camera);

	if (m_IsScenePlaying)
	{
		Nexus::ScriptEngine::OnSceneUpdate(ts.GetSeconds());
	}
}

void EditorLayer::OnRender()
{
	// Graphics
	{
		Nexus::Command::BeginRenderpass(m_GraphicsPass, m_GraphicsFramebuffer);

		Nexus::Command::BindPipeline(m_Pipeline);

		Nexus::Command::SetViewport(m_viewport);
		Nexus::Command::SetScissor(m_scissor);

		m_SceneRenderer.Render();

		Nexus::Command::EndRenderpass();
	}

	// ImGui
	{
		Nexus::Command::BeginRenderpass(m_ImGuiPass, m_ImGuiFramebuffer);
		Nexus::EditorContext::StartFrame();
		
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		m_SceneHeirarchy.Render();
		m_ImGuiEditorViewport->Render();

    ImGui::Begin("Script");
	  ImGui::Text("Hellooo Scripting !");

	  if (ImGui::Button("Start Scene"))
	  {
  		m_IsScenePlaying = true;
  		Nexus::ScriptEngine::OnSceneStart(m_Scene);
	  }

	  if (ImGui::Button("End Scene") && m_IsScenePlaying)
	  {
  		m_IsScenePlaying = false;
	  	Nexus::ScriptEngine::OnSceneStop();
	  }
	  ImGui::End();

		Nexus::EditorContext::Render();
		Nexus::Command::EndRenderpass();
	}
}

void EditorLayer::OnDetach()
{
	m_SceneData->Destroy();
	m_Scene->Clear();

	Nexus::EditorContext::Shutdown();
	NEXUS_LOG_DEBUG("Editor Layer Detached");
}

void EditorLayer::OnWindowResize(int width, int height)
{
	Nexus::Extent Extent = Nexus::Renderer::GetSwapchain()->GetExtent();
	
	// Screen
	{
		m_viewport.x = 0.0f;
		m_viewport.y = 0.0f;
		m_viewport.width = (float)Extent.width;
		m_viewport.height = (float)Extent.height;
		m_viewport.minDepth = 0.0f;
		m_viewport.maxDepth = 1.0f;

		m_scissor.Offset = { 0,0 };
		m_scissor.Extent = { Extent.width, Extent.height };

		m_cameraController.SetPerspectiveProjection(45.f, (float)width, (float)height, 0.1f, 1000.f);
	}

	m_GraphicsFramebuffer.reset();
	m_ImGuiFramebuffer.reset();

	m_GraphicsFBspecs.extent = Extent;
	m_GraphicsFramebuffer = Nexus::Framebuffer::Create(m_GraphicsFBspecs);

	m_ImGuiFBspecs.extent = Extent;
	m_ImGuiFramebuffer = Nexus::Framebuffer::Create(m_ImGuiFBspecs);

	m_ImGuiEditorViewport->SetContext(m_GraphicsFramebuffer, 2);
}