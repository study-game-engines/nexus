#include "nxpch.h"
#include "Mesh.h"

#include "Vertex.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#include "Renderer.h"
#include <glm/gtc/type_ptr.hpp>

Nexus::Ref<Nexus::StaticMesh> Nexus::StaticMesh::LoadWithAssimp(const char* Filepath)
{
	tinygltf::Model input;
	tinygltf::TinyGLTF context;
	std::string error, warning;

	bool loaded = context.LoadASCIIFromFile(&input, &error, &warning, Filepath);
	
	if (!warning.empty())
	{
		NEXUS_LOG_ERROR("TinyGLTF Warning: {0}", warning);
	}

	if (!error.empty())
	{
		NEXUS_LOG_ERROR("TinyGLTF Error: {0}", error);
		NEXUS_ASSERT(true, "Mesh Loading Error");
	}

	std::vector<StaticMeshVertex> m_Vertices;
	std::vector<uint32_t> m_Indices;

	for (auto& mesh : input.meshes)
	{
		for (auto& primitive : mesh.primitives)
		{
			uint32_t vertexStart = (uint32_t)m_Vertices.size();
			uint32_t indexStart = (uint32_t)m_Indices.size();

			// Vertices
			{
				const float* positionBuffer = nullptr;
				const float* normalBuffer = nullptr;
				const float* texCoordBuffer = nullptr;
				size_t vertexCount = 0;

				auto p = primitive.attributes.find("POSITION");
				if (p != primitive.attributes.end())
				{
					auto& accessor = input.accessors[p->second];
					auto& view = input.bufferViews[accessor.bufferView];
					positionBuffer = reinterpret_cast<const float*>(&input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]);

					vertexCount = accessor.count;
				}

				auto n = primitive.attributes.find("NORMAL");
				if (n != primitive.attributes.end())
				{
					auto& accessor = input.accessors[n->second];
					auto& view = input.bufferViews[accessor.bufferView];
					normalBuffer = reinterpret_cast<const float*>(&input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]);
				}

				auto t = primitive.attributes.find("TEXCOORD_0");
				if (t != primitive.attributes.end())
				{
					auto& accessor = input.accessors[t->second];
					auto& view = input.bufferViews[accessor.bufferView];
					texCoordBuffer = reinterpret_cast<const float*>(&input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]);
				}

				for (size_t i = 0; i < vertexCount; i++)
				{
					StaticMeshVertex vertex{};
					vertex.position = glm::make_vec3(&positionBuffer[i * 3]);

					if (normalBuffer)
						vertex.normal = glm::make_vec3(&normalBuffer[i * 3]);
					else
						vertex.normal = glm::vec3(0.f);

					if (texCoordBuffer)
						vertex.texCoord = glm::make_vec2(&texCoordBuffer[i * 2]);
					else
						vertex.texCoord = glm::vec2(0.f);

					m_Vertices.push_back(vertex);
				}
			}

			// Indices
			{
				auto& accessor = input.accessors[primitive.indices];
				auto& view = input.bufferViews[accessor.bufferView];
				auto& buffer = input.buffers[view.buffer];

				uint32_t indexCount = accessor.count;

				switch (accessor.componentType)
				{
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
					const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
					for (size_t index = 0; index < accessor.count; index++) {
						m_Indices.push_back(buf[index] + vertexStart);
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
					const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
					for (size_t index = 0; index < accessor.count; index++) {
						m_Indices.push_back(buf[index] + vertexStart);
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
					const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + view.byteOffset]);
					for (size_t index = 0; index < accessor.count; index++) {
						m_Indices.push_back(buf[index] + vertexStart);
					}
					break;
				}
				default:
					std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
					break;
				}
			}
		}
	}

	if (m_Vertices.empty() || m_Indices.empty())
	{
		std::string s = Filepath;
		s += " :: Is Empty";

		NEXUS_ASSERT(1, s.c_str());
	}

	NEXUS_LOG_TRACE("Mesh Loaded: {0} | vertices-{1} | indices-{2}", Filepath, m_Vertices.size(), m_Indices.size());

	Ref<StaticMesh> sMesh = CreateRef<StaticMesh>();

	sMesh->m_Vb = StaticBuffer::Create((uint32_t)m_Vertices.size() * sizeof(StaticMeshVertex), BufferType::Vertex, m_Vertices.data());
	sMesh->m_Ib = StaticBuffer::Create((uint32_t)m_Indices.size() * sizeof(uint32_t), BufferType::Index, m_Indices.data());

	Renderer::TransferMeshToGPU(sMesh);

	return sMesh;
}
