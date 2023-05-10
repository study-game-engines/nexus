#include "nxpch.h"
#include "JpWorld.h"
#include "JpEngine.h"
#include "JpBody.h"

#include "Scene/Entity.h"

Nexus::ObjectLayerPairFilterImpl Nexus::JoltPhysicsWorld::m_ObjLayerPairFilter;
Nexus::BPLayerInterfaceImpl Nexus::JoltPhysicsWorld::m_BpLayerInterface;
Nexus::ObjectVsBroadPhaseLayerFilterImpl Nexus::JoltPhysicsWorld::m_ObjVsBpLayerFilter;

Nexus::JoltPhysicsWorld::JoltPhysicsWorld()
{
	m_PhysicsSystem = new JPH::PhysicsSystem();
	m_PhysicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodies, cMaxContactConstraints,
		m_BpLayerInterface, m_ObjVsBpLayerFilter, m_ObjLayerPairFilter);

	m_BodyInterface = &m_PhysicsSystem->GetBodyInterface();
}

Nexus::JoltPhysicsWorld::~JoltPhysicsWorld()
{
	delete m_PhysicsSystem;
}

void Nexus::JoltPhysicsWorld::OnSceneStart(Ref<Scene> scene)
{
	m_ActiveEntities.clear();
	m_Bodies.clear();

	auto boxColliderView = scene->GetAllEntitiesWith<Component::BoxCollider>();
	for (auto& e : boxColliderView)
		CreateBodyWithEntity({ e,scene.get() });

	auto capsuleColliderView = scene->GetAllEntitiesWith<Component::CapsuleCollider>();
	for (auto& e : capsuleColliderView)
		CreateBodyWithEntity({ e,scene.get() });

	auto sphereColliderView = scene->GetAllEntitiesWith<Component::SphereCollider>();
	for (auto& e : sphereColliderView)
		CreateBodyWithEntity({ e,scene.get() });

	auto cylinderColliderView = scene->GetAllEntitiesWith<Component::CylinderCollider>();
	for (auto& e : cylinderColliderView)
		CreateBodyWithEntity({ e,scene.get() });

	m_PhysicsSystem->OptimizeBroadPhase();
}

void Nexus::JoltPhysicsWorld::OnSceneUpdate(float dt)
{
	m_PhysicsSystem->Update(dt, 1, 1, JoltPhysicsEngine::sInstance->m_Allocator, JoltPhysicsEngine::sInstance->m_JobSystem);

	for (auto& [k, v] : m_ActiveEntities)
	{
		JPH::BodyID id(k);
		auto pos = m_BodyInterface->GetPosition(id);
		auto rot = m_BodyInterface->GetRotation(id);

		auto& Transform = v.GetComponent<Component::Transform>();
		Transform.Translation = { pos.GetX(),pos.GetY(),pos.GetZ() };
		Transform.SetRotation({ rot.GetX(),rot.GetY(),rot.GetZ(),rot.GetW() });
	}
}

void Nexus::JoltPhysicsWorld::OnSceneStop()
{
	if (m_Bodies.empty())
		return;

	m_BodyInterface->RemoveBodies(m_Bodies.data(), (int)m_Bodies.size());
	m_BodyInterface->DestroyBodies(m_Bodies.data(), (int)m_Bodies.size());
}

void Nexus::JoltPhysicsWorld::SetGravity(const glm::vec3& gravity)
{
	m_PhysicsSystem->SetGravity(JoltUtils::ToJoltVector3(gravity));
}

void Nexus::JoltPhysicsWorld::CreateBodyWithEntity(Entity en)
{
	JoltBody body{};

	if (en.HasComponent<Component::RigidBody>())
	{
		body = JoltBodyFactor::CreateRigidBody(*m_BodyInterface, en);
		m_ActiveEntities[body.Id.GetIndexAndSequenceNumber()] = en;
	}
	else
	{
		body = JoltBodyFactor::CreateCollider(*m_BodyInterface, en);
	}
	m_Bodies.push_back(body.Id);
	m_BodyInterface->AddBody(body.Id, body.activationflag);
}
