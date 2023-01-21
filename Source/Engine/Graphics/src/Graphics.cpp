#include "Graphics/Graphics.h"
#include "Graphics/Backend.h"

void Nexus::Graphics::Engine::Initialize(const EngineSpecification& specs)
{
	Backend::Init(specs);
}

void Nexus::Graphics::Engine::Render()
{
}

void Nexus::Graphics::Engine::Shutdown()
{
	Backend::Shut();
}
