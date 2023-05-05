#include "nxpch.h"
#include "SceneHeirarchy.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "Core/FileDialog.h"
#include "Assets/AssetManager.h"

static void DrawVec3Control(const char* label, glm::vec3& vector, float reset = 0.f, float columnWidth = 100.f)
{
	ImGui::PushID(label);

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);

	ImGui::Text(label);
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0,0 });

	float lineheight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;
	ImVec2 buttonsize = { lineheight + 3.f,lineheight };

	auto& BoldFont = ImGui::GetIO().Fonts->Fonts[0];

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f,0.1f,0.1f,1.f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.7f,0.1f,0.1f,1.f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.6f,0.1f,0.1f,1.f });
	ImGui::PushFont(BoldFont);
	if (ImGui::Button("X", buttonsize))
		vector.x = reset;
	ImGui::PopStyleColor(3);
	ImGui::PopFont();

	ImGui::SameLine();
	ImGui::DragFloat("##X", &vector.x, 0.1f);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f,0.8f,0.1f,1.f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.1f,0.7f,0.1f,1.f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f,0.6f,0.1f,1.f });
	ImGui::PushFont(BoldFont);
	if (ImGui::Button("Y", buttonsize))
		vector.y = reset;
	ImGui::PopStyleColor(3);
	ImGui::PopFont();

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &vector.y, 0.1f);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f,0.1f,0.8f,1.f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.1f,0.1f,0.7f,1.f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f,0.1f,0.6f,1.f });
	ImGui::PushFont(BoldFont);
	if (ImGui::Button("Z", buttonsize))
		vector.z = reset;
	ImGui::PopStyleColor(3);
	ImGui::PopFont();

	ImGui::SameLine();
	ImGui::DragFloat("##Z", &vector.z, 0.1f);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PopStyleVar();

	ImGui::Columns(1);

	ImGui::PopID();
}

template <typename T>
static void DisplayAddComponentEntry(const std::string& entryName,Nexus::Entity e)
{
	if (!e.HasComponent<T>())
	{
		if (ImGui::MenuItem(entryName.c_str()))
		{
			e.AddComponent<T>();
			ImGui::CloseCurrentPopup();
		}
	}
}

template<typename T, typename UIFuntion>
static void DrawComponent(const std::string& name, Nexus::Entity e, UIFuntion uiFunction)
{
	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap
		| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

	if (e.HasComponent<T>())
	{
		auto& component = e.GetComponent<T>();

		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		ImGui::Separator();
		bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());

		ImGui::PopStyleVar();

		ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
		if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
		{
			ImGui::OpenPopup("ComponentSettings");
		}

		bool removeComponent = false;
		if (ImGui::BeginPopup("ComponentSettings"))
		{
			if (ImGui::MenuItem("Remove component"))
				removeComponent = true;

			ImGui::EndPopup();
		}

		if (open)
		{
			uiFunction(component);
			ImGui::TreePop();
		}

		if (removeComponent)
			e.RemoveComponent<T>();
	}
}

void Nexus::SceneHeirarchy::SetContext(Ref<Scene> scene)
{
	m_Scene = scene;
}

void Nexus::SceneHeirarchy::Render()
{
	{
		ImGui::Begin("Scene Heirarchy");

		m_Scene->m_registry.each([&](entt::entity e)
			{
				DrawEntityNode(e);
			}
		);

		if (ImGui::BeginPopupContextWindow(0, 1)) 
		{
			if (ImGui::MenuItem("Create Entity"))
				m_Scene->CreateEntity();
			ImGui::EndPopup();
		}
		
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_SelectedEntity = entt::null;
		}

		ImGui::End();
	}

	{
		ImGui::Begin("Properties");

		if (m_SelectedEntity != entt::null)
		{
			DrawComponents(m_SelectedEntity);
		}

		ImGui::End();
	}
}

void Nexus::SceneHeirarchy::DrawEntityNode(entt::entity e)
{
	auto& TagComponent = m_Scene->m_registry.get<Component::Tag>(e);

	ImGuiTreeNodeFlags flags = ((e == m_SelectedEntity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

	bool opened = ImGui::TreeNodeEx((void*)(uint64_t)e, flags, TagComponent.name.c_str());
	
	if (ImGui::IsItemClicked())
		m_SelectedEntity = e;

	bool Delete = false;
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Delete Entity"))
			Delete = true;
		ImGui::EndPopup();
	}

	if (opened)
		ImGui::TreePop();

	if (Delete)
	{
		m_Scene->DestroyEntity({ e,m_Scene.get() });

		if (m_SelectedEntity == e)
			m_SelectedEntity = entt::null;
	}
}

void Nexus::SceneHeirarchy::DrawComponents(entt::entity e)
{
	Entity en(e, m_Scene.get());

	{
		auto& Tag = en.GetComponent<Component::Tag>();

		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strncpy_s(buffer, Tag.name.c_str(), sizeof(buffer));

		if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
		{
			Tag.name = std::string(buffer);
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<Component::Mesh>("Mesh", en);
			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();
	}

	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

		if (en.HasComponent<Component::Transform>())
		{

			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(Component::Transform).hash_code(), treeNodeFlags, "Transform");
			ImGui::PopStyleVar();

			if (open)
			{
				auto& component = en.GetComponent<Component::Transform>();
				glm::vec3& rot = component.GetRotationEuler();

				DrawVec3Control("Translation", component.Translation);
				DrawVec3Control("Rotation", rot);
				DrawVec3Control("Scale", component.Scale, 1.f);
				ImGui::TreePop();
			}
		}
	}

	DrawComponent<Component::Mesh>("Mesh", {e,m_Scene.get()}, [&](auto& component)
		{
			AssetHandle handle = component.handle;
			auto& meshAsset = AssetManager::Get<StaticMeshAsset>(handle);
			ImGui::LabelText("MeshName", meshAsset.Name.c_str());
			ImGui::LabelText("MeshPath", meshAsset.Path.string().c_str());
			ImGui::SameLine();
			
			static std::string browsedString;
			if (ImGui::Button("Browse"))
			{
				std::string path = FileDialog::OpenFile("FBX Model (*.fbx)\0*.fbx\0");
				if (!path.empty())
				{
					browsedString = path;

				}
			}

			if (ImGui::Button("Set Mesh"))
			{
				AssetHandle newhandle = AssetManager::LoadFromFile<StaticMeshAsset>(browsedString);
				browsedString.clear();

				component.handle = newhandle;
			}
		});
}