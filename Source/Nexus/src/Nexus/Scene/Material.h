#pragma once
#include "Assets/Asset.h"
#include "Assets/Importer/glTFImporter.h"
#include "glm/glm.hpp"

namespace Nexus
{ 
	struct CombinedImageSampler
	{
		UUID Image = 0;
		UUID Sampler = 1;
		uint8_t TexCoord;
	};

	struct MaterialCreateInfo
	{
		CombinedImageSampler albedo;
		CombinedImageSampler metallicRoughness;
		CombinedImageSampler normal;
		glm::vec4 albedoColor;
		float roughness, metalness;
		float useMR, useAlb, useNormal;
	};

	class Material : public Asset
	{
		friend class SceneBuildData;
	public:
		static Ref<Material> Create(const MaterialCreateInfo& Info);

		Material() = default;
		~Material() override = default;

		bool IsUsingAlbedo() { return (useAlbedo == 1.f); }
		bool IsUsingMetallicRoughness() { return (useMR == 1.f); }
		bool IsUsingNormal() { return (useNormal == 1.f); }

		glm::vec4& GetAlbedoColor() { return m_AlbedoColor; }
		float& GetMetalness() { return m_metalness; }
		float& GetRoughness() { return m_roughness; }

		void SetUseAlbedo(bool use);
		void SetUseMetallicRoughness(bool use);
		void SetUseNormal(bool use);

		void SetAlbedoColor(const glm::vec4 color);
		void SetMetalness(float metalness);
		void SetRoughness(float roughness);
	private:
		CombinedImageSampler m_AlbedoMap;
		CombinedImageSampler m_MetallicRoughnessMap;
		CombinedImageSampler m_Normal;

		float useAlbedo = 0.f;
		float useMR = 0.f;
		float useNormal = 0.f;

		glm::vec4 m_AlbedoColor = glm::vec4(1.f);
		float m_metalness = 0.5f;
		float m_roughness = 0.5f;
	};
}