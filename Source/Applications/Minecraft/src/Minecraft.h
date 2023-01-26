#pragma once
#include "Core/Layer.h"

#include "Graphics/Framebuffer.h"
#include "Graphics/Renderpass.h"
#include "Graphics/Pipeline.h"

class Minecraft : public Nexus::Layer
{
public:
	void OnAttach() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDetach() override;
	void OnCallback() override;
private:
	void CreateAttachments();
	void CreateFramebuffers();
	void CreateRenderpasses();
	void CreatePipelines();
private:
	// Attachments
	std::vector<Nexus::Graphics::FramebufferAttachment> ColorAttachments;
	Nexus::Graphics::FramebufferAttachment DepthAttachment;

	// Framebuffers
	std::vector<Nexus::Graphics::Framebuffer> Framebuffers;

	// Renderpass
	Nexus::Graphics::Renderpass renderpass;

	// Pipelines 
	Nexus::Graphics::PipelineLayout pipelineLayout;
	Nexus::Graphics::GraphicsPipeline pipeline;

	// Clear Value
	std::vector<VkClearValue> clearValue;
};

