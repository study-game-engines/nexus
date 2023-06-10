#pragma once
#include "NxCore/Base.h"
#include "NxCore/Window.h"
#include "RendererAPI.h"

#include "NxGraphics/Context.h"
#include "NxGraphics/Swapchain.h"

namespace Nexus::GraphicsInterface
{
	Ref<Context> CreateContext(RendererAPI API, const ContextCreateInfo& Info);
	Ref<Swapchain> CreateSwapchain(Window* window);
}
