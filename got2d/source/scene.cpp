#include <algorithm>
#include "scene.h"
#include "engine.h"

Scene::Scene(float boundSize)
	: m_spatial(boundSize)
	, m_mouseButtonState{ 0, 1, 2 }
{
	//for main camera
	CreateCameraNode();

	RegisterKeyEventReceiver();
	RegisterMouseEventReceiver();
}

void Scene::RegisterKeyEventReceiver()
{
	m_keyPressReceiver.UserData
		= m_keyPressingBeginReceiver.UserData
		= m_keyPressingReceiver.UserData
		= m_keyPressingEndReceiver.UserData
		= this;

	m_keyPressReceiver.Functor = [](void* userData, g2d::KeyCode key)
	{
		::Scene* scene = reinterpret_cast<::Scene*>(userData);
		scene->OnKeyPress(key);
	};

	m_keyPressingBeginReceiver.Functor = [](void* userData, g2d::KeyCode key)
	{
		::Scene* scene = reinterpret_cast<::Scene*>(userData);
		scene->OnKeyPressingBegin(key);
	};

	m_keyPressingReceiver.Functor = [](void* userData, g2d::KeyCode key)
	{
		::Scene* scene = reinterpret_cast<::Scene*>(userData);
		scene->OnKeyPressing(key);
	};

	m_keyPressingEndReceiver.Functor = [](void* userData, g2d::KeyCode key)
	{
		::Scene* scene = reinterpret_cast<::Scene*>(userData);
		scene->OnKeyPressingEnd(key);
	};

	GetKeyboard().OnPress += m_keyPressReceiver;
	GetKeyboard().OnPressingBegin += m_keyPressingBeginReceiver;
	GetKeyboard().OnPressing += m_keyPressingReceiver;
	GetKeyboard().OnPressingEnd += m_keyPressingEndReceiver;
}

void Scene::RegisterMouseEventReceiver()
{
	m_mousePressReceiver.UserData
		= m_mousePressingBeginReceiver.UserData
		= m_mousePressingReceiver.UserData
		= m_mousePressingEndReceiver.UserData
		= m_mouseMovingReceiver.UserData
		= m_mouseDoubleClickReceiver.UserData
		= this;

	m_mousePressReceiver.Functor = [](void* userData, g2d::MouseButton button)
	{
		::Scene* scene = reinterpret_cast<::Scene*>(userData);
		scene->OnMousePress(button);
	};

	m_mousePressingBeginReceiver.Functor = [](void* userData, g2d::MouseButton button)
	{
		::Scene* scene = reinterpret_cast<::Scene*>(userData);
		scene->OnMousePressingBegin(button);
	};

	m_mousePressingReceiver.Functor = [](void* userData, g2d::MouseButton button)
	{
		::Scene* scene = reinterpret_cast<::Scene*>(userData);
		scene->OnMousePressing(button);
	};

	m_mousePressingEndReceiver.Functor = [](void* userData, g2d::MouseButton button)
	{
		::Scene* scene = reinterpret_cast<::Scene*>(userData);
		scene->OnMousePressingEnd(button);
	};

	m_mouseMovingReceiver.Functor = [](void* userData, g2d::MouseButton button)
	{
		::Scene* scene = reinterpret_cast<::Scene*>(userData);
		scene->OnMouseMoving();
	};

	m_mouseDoubleClickReceiver.Functor = [](void* userData, g2d::MouseButton button)
	{
		::Scene* scene = reinterpret_cast<::Scene*>(userData);
		scene->OnMouseDoubleClick(button);
	};

	GetMouse().OnPress += m_mousePressReceiver;
	GetMouse().OnPressingBegin += m_mousePressingBeginReceiver;
	GetMouse().OnPressing += m_mousePressingReceiver;
	GetMouse().OnPressingEnd += m_mousePressingEndReceiver;
	GetMouse().OnMoving += m_mouseMovingReceiver;
	GetMouse().OnDoubleClick += m_mouseDoubleClickReceiver;
}

void Scene::ResortCameraOrder()
{
	if (m_cameraOrderDirty)
	{
		m_cameraOrderDirty = false;
		m_cameraOrder = m_cameras;

		std::sort(m_cameraOrder.begin(), m_cameraOrder.end(),
			[](g2d::Camera* a, g2d::Camera* b)->bool {

			auto aOrder = a->GetRenderingOrder();
			auto bOrder = b->GetRenderingOrder();
			if (aOrder == bOrder)
			{
				return a->GetID() < b->GetID();
			}
			else
			{
				return aOrder < bOrder;
			}
		});
	}
}

void Scene::ResetRenderingOrder()
{
	if (m_renderingOrderDirtyNode != nullptr)
	{
		m_renderingOrderDirtyNode->AdjustRenderingOrder();
		m_renderingOrderDirtyNode = nullptr;
	}
}

void Scene::Release()
{
	UnRegisterKeyEventReceiver();
	UnRegisterMouseEventReceiver();
	::GetEngineImpl()->RemoveScene(*this);
	// clean the children first, so that
	// child nodes can access SpatialGraph
	// in the scene
	m_children.ClearChildren();
	delete this;
}

void Scene::UnRegisterKeyEventReceiver()
{
	GetKeyboard().OnPress -= m_keyPressReceiver;
	GetKeyboard().OnPressingBegin -= m_keyPressingBeginReceiver;
	GetKeyboard().OnPressing -= m_keyPressingReceiver;
	GetKeyboard().OnPressingEnd -= m_keyPressingEndReceiver;
}

void Scene::UnRegisterMouseEventReceiver()
{
	GetMouse().OnPress -= m_mousePressReceiver;
	GetMouse().OnPressingBegin -= m_mousePressingBeginReceiver;
	GetMouse().OnPressing -= m_mousePressingReceiver;
	GetMouse().OnPressingEnd -= m_mousePressingEndReceiver;
	GetMouse().OnMoving -= m_mouseMovingReceiver;
	GetMouse().OnDoubleClick -= m_mouseDoubleClickReceiver;
}

void Scene::Update(uint32_t elapsedTime, uint32_t deltaTime)
{
	if (m_hoverNode != nullptr && m_canTickHovering && GetMouse().IsFree())
	{
		m_hoverNode->OnCursorHovering();
		m_canTickHovering = false;
	}

	m_children.OnUpdate(deltaTime);

	//TODO: checking whether m_Hovering is deleted
	m_canTickHovering = true;
}

void Scene::OnMessage(const g2d::Message& message, uint32_t currentTimeStamp)
{
	m_children.OnMessage(message);
}

void Scene::SetRenderingOrderDirty(::SceneNode* parent)
{
	if (m_renderingOrderDirtyNode == nullptr)
	{
		m_renderingOrderDirtyNode = parent;
	}
	else if (m_renderingOrderDirtyNode->GetRenderingOrder() > parent->GetRenderingOrder())
	{
		m_renderingOrderDirtyNode = parent;
	}
}

void Scene::OnResize()
{
	for (auto& camera : m_cameras)
	{
		camera->OnUpdateMatrixChanged();
	}
}

void Scene::AdjustRenderingOrder()
{
	m_renderingOrderEnd = 1;
	m_children.Traversal([&](::SceneNode* child)
	{
		child->SetRenderingOrder(m_renderingOrderEnd);
	});
}

void Scene::OnRemoveSceneNode(::SceneNode& node)
{
	if (&node == m_hoverNode)
	{
		m_hoverNode = nullptr;
	}

	for (auto& button : m_mouseButtonState)
	{
		button.OnRemoveSceneNode(node);
	}

	for (auto& camera : m_cameras)
	{
		camera->OnRemoveSceneNode(node);
	}
}

g2d::SceneNode * Scene::CreateChild()
{
	auto child = m_children.CreateChild(*this, m_children);
	child->SetRenderingOrder(m_renderingOrderEnd);
	return child;
}

void Scene::OnKeyPress(g2d::KeyCode key)
{
	m_children.OnKeyPress(key);
}

void Scene::OnKeyPressingBegin(g2d::KeyCode key)
{
	m_children.OnKeyPressingBegin(key);
}

void Scene::OnKeyPressing(g2d::KeyCode key)
{
	m_children.OnKeyPressing(key);
}

void Scene::OnKeyPressingEnd(g2d::KeyCode key)
{
	m_children.OnKeyPressingEnd(key);
}

void Scene::OnMousePress(g2d::MouseButton button)
{
	if (m_hoverNode != nullptr)
	{
		m_hoverNode->OnClick(button);
	}
}

void Scene::MouseButtonState::OnPressingBegin(::SceneNode* hitNode)
{
	if (hitNode != nullptr)
	{
		m_draggingNode = hitNode;
		m_draggingNode->OnDragBegin(Button);
	}
}

void Scene::OnMousePressingBegin(g2d::MouseButton button)
{
	m_mouseButtonState[(int)button].OnPressingBegin(m_hoverNode);
}

void Scene::MouseButtonState::OnPressing(::SceneNode* hitNode)
{
	//*  heart-beaten?
	if (m_draggingNode != nullptr)
	{
		if (hitNode != nullptr && hitNode != m_draggingNode)
		{
			m_draggingNode->OnDropping(hitNode, Button);
		}
		else
		{
			m_draggingNode->OnDragging(Button);
		}
	}
}

void Scene::OnMousePressing(g2d::MouseButton button)
{
	m_mouseButtonState[(int)button].OnPressing(m_hoverNode);
}

void Scene::MouseButtonState::OnPressingEnd(::SceneNode* hitNode)
{
	if (m_draggingNode != nullptr)
	{
		if (hitNode == nullptr && hitNode != m_draggingNode)
		{
			m_draggingNode->OnDropTo(hitNode, Button);
			hitNode->OnCursorEnterFrom(m_draggingNode);
		}
		else
		{
			m_draggingNode->OnDragEnd(Button);
			m_draggingNode->OnCursorEnterFrom(nullptr);
		}
		m_draggingNode = nullptr;
	}
}

void Scene::MouseButtonState::OnRemoveSceneNode(::SceneNode & node)
{
	if (&node == m_draggingNode)
	{
		m_draggingNode = nullptr;
	}
}

void Scene::OnMousePressingEnd(g2d::MouseButton button)
{
	m_mouseButtonState[(int)button].OnPressingEnd(m_hoverNode);
}

void Scene::OnMouseDoubleClick(g2d::MouseButton button)
{
	if (m_hoverNode != nullptr)
	{
		m_hoverNode->OnDoubleClick(button);
	}
}

void Scene::MouseButtonState::OnMoving(::SceneNode* hitNode)
{
	if (m_draggingNode != nullptr)
	{
		if (hitNode != nullptr && hitNode != m_draggingNode)
		{
			m_draggingNode->OnDropping(hitNode, Button);
		}
		else
		{
			m_draggingNode->OnDragging(Button);
		}
	}
}

void Scene::OnMouseMoving()
{
	::SceneNode* hitNode = FindInteractiveObject(GetMouse().GetCursorPosition());
	if (GetMouse().IsFree())
	{
		if (m_hoverNode != hitNode)
		{
			if (m_hoverNode != nullptr)
			{
				m_hoverNode->OnCursorLeaveTo(hitNode);
			}
			if (hitNode != nullptr)
			{
				hitNode->OnCursorEnterFrom(m_hoverNode);
			}
			m_hoverNode = hitNode;
		}
		else if (m_hoverNode != nullptr && m_canTickHovering)
		{
			m_hoverNode->OnCursorHovering();
			m_canTickHovering = false;
		}
	}
	else
	{
		for (auto& mouseButton : m_mouseButtonState)
		{
			mouseButton.OnMoving(hitNode);
		}
	}
}

::SceneNode* Scene::FindInteractiveObject(const gml::coord& cursorPos)
{
	auto cur = m_cameraOrder.rbegin();
	auto end = m_cameraOrder.rend();

	for (; cur != end; cur++)
	{
		auto& camera = *cur;
		gml::vec2 worldCoord = camera->ScreenToWorld(cursorPos);
		auto component = camera->FindNearestComponent(worldCoord);
		if (component != nullptr)
		{
			return reinterpret_cast<::SceneNode*>(component->GetSceneNode());
		}
	}
	return nullptr;
}

#include "render_system.h"

bool RenderingOrderSorter(g2d::Component* a, g2d::Component* b)
{
	return a->GetRenderingOrder() < b->GetRenderingOrder();
}

void Scene::Render()
{
	GetRenderSystem()->FlushRequests();
	ResortCameraOrder();
	ResetRenderingOrder();
	for (auto camera : m_cameraOrder)
	{
		if (!camera->IsActivity())
			continue;

		GetRenderSystem()->SetViewMatrix(camera->GetViewMatrix());

		camera->visibleComponents.clear();
		m_spatial.FindVisible(*camera);

		//sort visibleEntities by render order
		std::sort(
			camera->visibleComponents.begin(),
			camera->visibleComponents.end(),
			RenderingOrderSorter);

		for (auto& component : camera->visibleComponents)
		{
			component->OnRender();
		}
		GetRenderSystem()->FlushRequests();
	}
}

g2d::Camera* Scene::CreateCameraNode()
{
	Camera* camera = new ::Camera(*this, static_cast<uint32_t>(m_cameras.size()));
	m_cameras.push_back(camera);
	m_cameraOrderDirty = true;

	CreateChild()->AddComponent(camera, true);
	return camera;
}

g2d::Camera* Scene::GetCameraByIndex(uint32_t index) const
{
	ENSURE(index < m_cameras.size());
	return m_cameras[index];
}

uint32_t Scene::GetCameraCount() const
{
	return static_cast<uint32_t>(m_cameras.size());
}
