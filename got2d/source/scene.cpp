#include "spatial_graph.h"
#include "scene.h"
#include <algorithm>

Scene::Scene(float boundSize)
	: m_spatial(boundSize)
	, m_mouseButtonState{ g2d::MouseButton::Left, g2d::MouseButton::Right, g2d::MouseButton::Middle }
{
	CreateCameraNode();
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

void Scene::Release()
{
	EmptyChildren();
	delete this;
}

void Scene::Update(uint32_t elapsedTime)
{
	m_lastTickStamp += elapsedTime;
	for (auto& state : m_mouseButtonState)
	{
		state.Update(m_lastTickStamp);
	}

	//m_hoverNode->OnMouseHovering(elapsedTime);
	_Update(elapsedTime);
	AdjustRenderingOrder();
}

void Scene::AdjustRenderingOrder()
{
	uint32_t curIndex = 1;

	TraversalChildren([&](::SceneNode* child)
	{
		child->SetRenderingOrder(curIndex);
	});
}

void Scene::MouseButtonState::Update(uint32_t currentStamp)
{
	if (pressing && !dragging &&
		hoverNode != nullptr &&
		(currentStamp - pressStamp > PressingDelta))
	{
		dragging = true;
		// 使用上一个消息的按键
		hoverNode->OnDragBegin(button, cursorPos, control, shift, alt);
	}
}

void Scene::MouseButtonState::LostFocus(::SceneNode* hitNode)
{
	if (dragging)
	{
		if (hoverNode == hitNode)
		{
			hoverNode->OnDragEnd(button, cursorPos, control, shift, alt);
		}
		else
		{
			hoverNode->OnDropTo(hitNode, button, cursorPos, control, shift, alt);
		}

		dragging = false;
		pressing = false;
	}
	else if (pressing)
	{
		if (hoverNode == hitNode)
		{
			hoverNode->OnClick(button, cursorPos, control, shift, alt);
		}
		else
		{
			if (hitNode != nullptr)
			{
				hitNode->OnMouseEnterFrom(hoverNode, cursorPos, control, shift, alt);
			}
			hoverNode->OnMouseLeaveTo(hitNode, cursorPos, control, shift, alt);
		}
		pressing = false;
	}
	hoverNode = hitNode;
}

bool Scene::MouseButtonState::UpdateMessage(const g2d::Message& message, uint32_t currentStamp, ::SceneNode* hitNode)
{	//临时的记录上一个消息的按键
	control = message.Control;
	shift = message.Shift;
	alt = message.Alt;
	bool sameButton = message.MouseButton == button;

	if (message.Event == g2d::MessageEvent::MouseButtonDoubleClick && sameButton)
	{
		if (hitNode != nullptr)
		{
			hoverNode->OnDoubleClick(message.MouseButton,
				gml::coord(message.MousePositionX, message.MousePositionY),
				message.Control, message.Shift, message.Alt);
		}
		hoverNode = hitNode;
	}
	else if (message.Event == g2d::MessageEvent::MouseButtonDown && sameButton)
	{
		if (dragging)//异常状态
		{
			// 让上次的dragging事件正常结束
			hoverNode->OnDragEnd(button,
				gml::coord(message.MousePositionX, message.MousePositionY),
				message.Control, message.Shift, message.Alt);
			dragging = false;
			pressing = false;
		}
		else if (pressing)//异常状态
		{
			// 取消上次的点击操作
			pressing = false;
		}

		hoverNode = hitNode;
		if (hoverNode != nullptr)
		{
			pressing = true;
			pressStamp = currentStamp;
			cursorPos.set(message.MousePositionX, message.MousePositionY);
		}
		return true;
	}
	else if (message.Event == g2d::MessageEvent::MouseButtonUp && sameButton)
	{
		if (message.MouseButton != button)
			return false;

		if (dragging)
		{
			if (hitNode != hoverNode)
			{
				hoverNode->OnDropTo(hitNode, button,
					gml::coord(message.MousePositionX, message.MousePositionY),
					message.Control, message.Shift, message.Alt);
				if (hitNode)
				{
					hitNode->OnMouseEnterFrom(hoverNode,
						gml::coord(message.MousePositionX, message.MousePositionY),
						message.Control, message.Shift, message.Alt);
				}
				hoverNode = hitNode;
			}
			else
			{
				hoverNode->OnDragEnd(button,
					gml::coord(message.MousePositionX, message.MousePositionY),
					message.Control, message.Shift, message.Alt);
			}
			pressing = false;
			dragging = false;
			return true;
		}
		else if (pressing)
		{
			// 不相等的状态已经在MouseMove中解决了
			ENSURE(hitNode == hoverNode);
			hoverNode->OnClick(button,
				gml::coord(message.MousePositionX, message.MousePositionY),
				message.Control, message.Shift, message.Alt);
			pressing = false;
			return true;
		}
	}
	else if (message.Event == g2d::MessageEvent::MouseMove)
	{
		if (!dragging && pressing)
		{
			dragging = true;
			hoverNode->OnDragBegin(button,
				gml::coord(message.MousePositionX, message.MousePositionY),
				message.Control, message.Shift, message.Alt);
		}

		if (dragging)
		{
			if (hitNode != nullptr)
			{
				hoverNode->OnDropping(hitNode, button,
					gml::coord(message.MousePositionX, message.MousePositionY),
					message.Control, message.Shift, message.Alt);
			}
			else
			{
				hoverNode->OnDragging(button,
					gml::coord(message.MousePositionX, message.MousePositionY),
					message.Control, message.Shift, message.Alt);
			}
			return true;
		}
	}
	return false;
}

void Scene::OnMessage(const g2d::Message& message)
{
	TraversalChildren([&](::SceneNode* child) {
		child->OnMessage(message);
	});

	::SceneNode* hitNode = FindInteractiveObject(message);
	if (message.Event == g2d::MessageEvent::LostFocus)
	{
		for (auto& state : m_mouseButtonState)
			state.LostFocus(hitNode);
	}
	else if (message.Source == g2d::MessageSource::Mouse)
	{
		bool handleMove = false;
		for (auto& state : m_mouseButtonState)
		{
			if (state.UpdateMessage(message, m_lastTickStamp, hitNode))
			{
				handleMove = true;
			}
		}

		if (!handleMove && message.Event == g2d::MessageEvent::MouseMove)
		{
			if (m_hoverNode != hitNode)
			{
				if (m_hoverNode != nullptr)
				{
					m_hoverNode->OnMouseLeaveTo(hitNode,
						gml::coord(message.MousePositionX, message.MousePositionY),
						message.Control, message.Shift, message.Alt);
				}
				if (hitNode != nullptr)
				{
					hitNode->OnMouseEnterFrom(m_hoverNode,
						gml::coord(message.MousePositionX, message.MousePositionY),
						message.Control, message.Shift, message.Alt);
				}
				m_hoverNode = hitNode;
			}
			else
			{
				// move inside 
				// dont care
			}
		}
	}
}

::SceneNode* Scene::FindInteractiveObject(const g2d::Message& message)
{
	auto cur = std::rbegin(m_cameraOrder);
	auto end = std::rend(m_cameraOrder);
	g2d::Entity* frontEntity = nullptr;
	for (; cur != end; cur++)
	{
		::Camera* camera = *cur;

		int coordX = message.MousePositionX;
		int coordY = message.MousePositionY;
		gml::vec2 worldCoord = camera->ScreenToWorld({ coordX, coordY });
		auto entity = camera->FindIntersectionObject(worldCoord);
		if (entity != nullptr)
		{
			return reinterpret_cast<::SceneNode*>(entity->GetSceneNode());
		}
	}
	return nullptr;
}

#include "render_system.h"
void Scene::Render()
{
	GetRenderSystem()->FlushRequests();
	ResortCameraOrder();
	for (auto camera : m_cameraOrder)
	{
		if (camera->IsActivity())
		{
			camera->visibleEntities.clear();
			GetRenderSystem()->SetViewMatrix(camera->GetViewMatrix());
			m_spatial.FindVisible(*camera);

			//sort visibleEntities by render order
			std::sort(std::begin(camera->visibleEntities), std::end(camera->visibleEntities),
				[](g2d::Entity* a, g2d::Entity* b) {
				return a->GetRenderingOrder() < b->GetRenderingOrder();
			});

			for (auto& entity : camera->visibleEntities)
			{
				entity->OnRender();
			}
			GetRenderSystem()->FlushRequests();
		}
	}
}

g2d::Camera* Scene::CreateCameraNode()
{
	Camera* camera = new ::Camera();
	CreateSceneNodeChild(camera, true);
	m_cameraOrderDirty = true;
	camera->SetID(static_cast<uint32_t>(m_cameras.size()));
	m_cameras.push_back(camera);
	return camera;
}

g2d::Camera* Scene::GetCameraByIndex(uint32_t index) const
{
	ENSURE(index < m_cameras.size());
	return m_cameras[index];
}