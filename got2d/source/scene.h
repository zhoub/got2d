#pragma once
#include "../include/g2dscene.h"
#include "component.h"
#include "spatial_graph.h"
#include "input.h"
#include <gmlmatrix.h>
#include <vector>

class SceneNode;
class Scene;

struct NodeComponent
{
	NodeComponent(g2d::Component* c, bool ar) : ComponentPtr(c), AutoRelease(ar) { }
	g2d::Component* ComponentPtr = nullptr;
	bool AutoRelease = false;
};

class ComponentContainer
{
public:
	~ComponentContainer();

	void Collect();

	bool Add(g2d::SceneNode* parent, g2d::Component* component, bool autoRelease);

	bool Remove(g2d::Component* component, bool forceNotReleased);

	bool IsAutoRelease(g2d::Component* component) const;

	g2d::Component* At(uint32_t index) const;

	uint32_t GetCount() const { return static_cast<uint32_t>(m_components.size()); }

	template<typename FUNC> void Traversal(FUNC f)
	{
		for (auto& c : m_components) f(c.ComponentPtr);
	}

public:
	void OnRotate(gml::radian r);

	void OnScale(const gml::vec2& s);

	void OnMove(const gml::vec2& p);

	void OnMessage(const g2d::Message& message);

	void OnUpdate(uint32_t deltaTime);

	void OnUpdateMatrixChanged();

	void OnCursorEnterFrom(::SceneNode* adjacency);

	void OnCursorHovering();

	void OnCursorLeaveTo(::SceneNode* adjacency);

	void OnLClick();

	void OnRClick();

	void OnMClick();

	void OnLDoubleClick();

	void OnRDoubleClick();

	void OnMDoubleClick();

	void OnLDragBegin();

	void OnRDragBegin();

	void OnMDragBegin();

	void OnLDragging();

	void OnRDragging();

	void OnMDragging();

	void OnLDragEnd();

	void OnRDragEnd();

	void OnMDragEnd();

	void OnLDropping(::SceneNode* dropped);

	void OnRDropping(::SceneNode* dropped);

	void OnMDropping(::SceneNode* dropped);

	void OnLDropTo(::SceneNode* dropped);

	void OnRDropTo(::SceneNode* dropped);

	void OnMDropTo(::SceneNode* dropped);

	void OnKeyPress(g2d::KeyCode key);

	void OnKeyPressingBegin(g2d::KeyCode key);

	void OnKeyPressing(g2d::KeyCode key);

	void OnKeyPressingEnd(g2d::KeyCode key);

private:
	void Recollect();

	void DelayRemove();

	std::vector<NodeComponent> m_components;
	std::vector<NodeComponent> m_released;
	std::vector<NodeComponent> m_collection;
	bool m_collectionChanged = true;
};

class SceneNodeContainer
{
public:
	~SceneNodeContainer();

	void Collect();

	void ClearChildren();

	::SceneNode* CreateChild(::Scene& scene, ::SceneNode& parent);

	::SceneNode* CreateChild(::Scene& scene, SceneNodeContainer& parent);

	::SceneNode* At(uint32_t index) const;

	::SceneNode* First() const;

	::SceneNode* Last() const;

	uint32_t GetCount() const { return static_cast<uint32_t>(m_children.size()); }

	void Remove(::SceneNode& child);

	bool Move(uint32_t from, uint32_t to);

	template<typename FUNC> void Traversal(FUNC func)
	{
		for (auto& child : m_children) func(child);
	}

	template<typename FUNC> void InverseTraversal(FUNC func)
	{
		auto it = std::rbegin(m_children);
		auto end = std::rend(m_children);
		for (; it != end; it++) func(*child);
	}

public:
	void OnMessage(const g2d::Message& message);

	void OnUpdate(uint32_t deltaTime);

	void OnKeyPress(g2d::KeyCode key);

	void OnKeyPressingBegin(g2d::KeyCode key);

	void OnKeyPressing(g2d::KeyCode key);

	void OnKeyPressingEnd(g2d::KeyCode key);

private:
	void Recollect();

	void DelayRemove();

	std::vector<::SceneNode*> m_children;
	std::vector<::SceneNode*> m_released;
	std::vector<::SceneNode*> m_collection;
	bool m_collectionChanged = true;
};

class LocalTransform
{
public:
	LocalTransform();

	void SetPosition(const gml::vec2& position);

	void SetPivot(const gml::vec2& pivot);

	void SetScale(const gml::vec2& scale);

	void SetRotation(gml::radian r);

	const gml::mat32& GetMatrix();

	const gml::vec2& GetPosition() const { return m_position; }

	const gml::vec2& GetPivot() const { return m_pivot; }

	const gml::vec2& GetScale() const { return m_scale; }

	gml::radian GetRotation() const { return m_rotation; }

private:
	void SetMatrixDirty();

	gml::vec2 m_position;
	gml::vec2 m_pivot;
	gml::vec2 m_scale;
	gml::radian m_rotation;
	gml::mat32 m_matrix;
	bool m_matrixDirty = true;
};

class WorldTransform
{
public:
	WorldTransform(::SceneNode& node, LocalTransform& local);

	const gml::vec2& GetPosition();

	const gml::mat32& GetMatrix();

	void SetMatrixDirty() { m_matrixDirty = true; }

	void SetPositionDirty() { m_positionDirty = true; }

private:
	::SceneNode& m_sceneNode;
	LocalTransform& m_localTransform;
	gml::vec2 m_position;
	gml::mat32 m_matrix;
	bool m_matrixDirty = true;
	bool m_positionDirty = true;
};
class SceneNode : public g2d::SceneNode
{
	RTTI_IMPL;
public:
	SceneNode(::Scene& scene, ::SceneNode* parent, uint32_t childID);

	SceneNode(::Scene& scene, SceneNodeContainer& parentContainer, uint32_t childID);

	~SceneNode();

	uint32_t GetRenderingOrder() const { return m_renderingOrder; }

	::SceneNode* GetParent() const { return m_parent; }

	void AdjustRenderingOrder();

	void OnUpdate(uint32_t deltaTime);

	void SetChildIndex(uint32_t index) { m_childIndex = index; }

	void SetRenderingOrder(uint32_t& order);

	//提供给Move函数临时交换用
	void SetRenderingOrderOnly(uint32_t order);

	bool ParentIsScene() const { return m_parent == nullptr; }

	void OnMessage(const g2d::Message& message);

	void OnCursorEnterFrom(::SceneNode* adjacency);

	void OnCursorHovering();

	void OnCursorLeaveTo(::SceneNode* adjacency);

	void OnClick(g2d::MouseButton button);

	void OnDoubleClick(g2d::MouseButton button);

	void OnDragBegin(g2d::MouseButton button);

	void OnDragging(g2d::MouseButton button);

	void OnDragEnd(g2d::MouseButton button);

	void OnDropping(::SceneNode* dropped, g2d::MouseButton button);

	void OnDropTo(::SceneNode* dropped, g2d::MouseButton button);

	void OnKeyPress(g2d::KeyCode key);

	void OnKeyPressingBegin(g2d::KeyCode key);

	void OnKeyPressing(g2d::KeyCode key);

	void OnKeyPressingEnd(g2d::KeyCode key);

public:	//g2d::SceneNode
	virtual void Release() override;

	virtual g2d::Scene* GetScene() const override;

	virtual g2d::SceneNode* GetParentNode() const override { return m_parent; }

	virtual g2d::SceneNode* GetPrevSiblingNode() const override { return GetPrevSibling(); }

	virtual g2d::SceneNode* GetNextSiblingNode() const override { return GetNextSibling(); }

	virtual g2d::SceneNode* FirstChild() const override { return m_children.First(); }

	virtual g2d::SceneNode* LastChild() const override { return m_children.Last(); }

	virtual g2d::SceneNode* GetChildByIndex(uint32_t index) const override { return m_children.At(index); }

	virtual uint32_t GetChildCount() const override { return m_children.GetCount(); }

	virtual g2d::SceneNode* CreateChild() override;

	virtual void MoveToFront() override;

	virtual void MoveToBack() override;

	virtual void MovePrev() override;

	virtual void MoveNext() override;

	virtual bool AddComponent(g2d::Component* component, bool autoRelease) override;

	virtual bool RemoveComponent(g2d::Component* component) override;

	virtual bool RemoveComponentWithoutRelease(g2d::Component* component) override;

	virtual bool IsComponentAutoRelease(g2d::Component* component) const override;

	virtual g2d::Component* GetComponentByIndex(uint32_t index) const override;

	virtual uint32_t GetComponentCount() const override { return m_components.GetCount(); }

	virtual const gml::mat32& GetLocalMatrix() override { return m_localTransform.GetMatrix(); }

	virtual const gml::mat32& GetWorldMatrix() override;

	virtual g2d::SceneNode* SetPosition(const gml::vec2& position) override;

	virtual g2d::SceneNode* SetWorldPosition(const gml::vec2& position) override;

	virtual g2d::SceneNode* SetPivot(const gml::vec2& pivot) override;

	virtual g2d::SceneNode* SetScale(const gml::vec2& scale) override;

	virtual g2d::SceneNode* SetRotation(gml::radian r) override;

	virtual void SetVisible(bool visible) override { m_isVisible = visible; }

	virtual void SetStatic(bool s) override;

	virtual void SetVisibleMask(uint32_t mask, bool recursive) override;

	virtual const gml::vec2& GetPosition()  const override { return m_localTransform.GetPosition(); }

	virtual const gml::vec2& GetPivot() const override { return m_localTransform.GetPivot(); }

	virtual const gml::vec2& GetScale() const override { return m_localTransform.GetScale(); }

	virtual gml::radian GetRotation() const override { return m_localTransform.GetRotation(); }

	virtual gml::vec2 GetWorldPosition() override;

	virtual uint32_t GetChildIndex() const override { return m_childIndex; }

	virtual bool IsVisible() const override { return m_isVisible; }

	virtual bool IsStatic() const override { return m_isStatic; }

	virtual uint32_t GetVisibleMask() const override { return m_visibleMask; }

	virtual gml::vec2 WorldToLocal(const gml::vec2& pos) override;

	virtual gml::vec2 WorldToParent(const gml::vec2& pos) override;

private:
	::SceneNode* GetPrevSibling() const;

	::SceneNode* GetNextSibling() const;

	void SetWorldMatrixDirty();

	void AdjustSpatial();

private:
	::Scene& m_scene;
	::SceneNode* m_parent = nullptr;
	SceneNodeContainer& m_parentContainer;
	SceneNodeContainer m_children;
	ComponentContainer m_components;
	LocalTransform m_localTransform;
	WorldTransform m_worldTransform;
	bool m_isVisible = true;
	bool m_isStatic = false;
	bool m_matrixDirtyUpdate = true;
	uint32_t m_childIndex = 0;
	uint32_t m_renderingOrder = 0xFFFFFFFF;//保证一开始是错误的
	uint32_t m_visibleMask = g2d::DEF_VISIBLE_MASK;

};

class Scene : public g2d::Scene
{
	RTTI_IMPL;
public:
	Scene(float boundSize);

	void SetRenderingOrderDirty(::SceneNode* parent);

	void SetCameraOrderDirty() { m_cameraOrderDirty = true; }

	void Update(uint32_t elapsedTime, uint32_t deltaTime);

	void OnMessage(const g2d::Message& message, uint32_t currentTimeStamp);

	void OnResize();

	SpatialGraph& GetSpatialGraph() { return m_spatial; }

	void AdjustRenderingOrder();

public: //g2d::Scene

	virtual void Release() override;

	virtual g2d::SceneNode* FirstChild() const override { return m_children.First(); }

	virtual g2d::SceneNode* LastChild() const override { return m_children.Last(); }

	virtual g2d::SceneNode* GetChildByIndex(uint32_t index) const override { return m_children.At(index); }

	virtual uint32_t GetChildCount() const override { return m_children.GetCount(); }

	virtual g2d::SceneNode* CreateChild() override;

	virtual g2d::Camera* CreateCameraNode() override;

	virtual g2d::Camera* GetMainCamera() const override { return GetCameraByIndex(0); }

	virtual g2d::Camera* GetCameraByIndex(uint32_t) const override;

	virtual uint32_t GetCameraCount() const override;

	virtual void Render() override;

private:
	void ResortCameraOrder();

	void ResetRenderingOrder();

	::SceneNode* FindInteractiveObject(const gml::coord& cursorPos);

	void RegisterKeyEventReceiver();

	void UnRegisterKeyEventReceiver();

	void RegisterMouseEventReceiver();

	void UnRegisterMouseEventReceiver();

	void OnKeyPress(g2d::KeyCode key);

	void OnKeyPressingBegin(g2d::KeyCode key);

	void OnKeyPressing(g2d::KeyCode key);

	void OnKeyPressingEnd(g2d::KeyCode key);

	void OnMousePress(g2d::MouseButton button);

	void OnMousePressingBegin(g2d::MouseButton button);

	void OnMousePressing(g2d::MouseButton button);

	void OnMousePressingEnd(g2d::MouseButton button);

	void OnMouseDoubleClick(g2d::MouseButton button);

	void OnMouseMoving();

	KeyEventReceiver m_keyPressReceiver;
	KeyEventReceiver m_keyPressingBeginReceiver;
	KeyEventReceiver m_keyPressingReceiver;
	KeyEventReceiver m_keyPressingEndReceiver;

	MouseEventReceiver m_mousePressReceiver;
	MouseEventReceiver m_mousePressingBeginReceiver;
	MouseEventReceiver m_mousePressingReceiver;
	MouseEventReceiver m_mousePressingEndReceiver;
	MouseEventReceiver m_mouseMovingReceiver;
	MouseEventReceiver m_mouseDoubleClickReceiver;

	class MouseButtonState
	{
		::SceneNode* dragNode = nullptr;
	public:
		const g2d::MouseButton Button;
		MouseButtonState(int index) : Button((g2d::MouseButton)index) { }
		void OnMoving(::SceneNode* hitNode);
		void OnPressingBegin(::SceneNode* hitNode);
		void OnPressing(::SceneNode* hitNode);
		void OnPressingEnd(::SceneNode* hitNode);
	} m_mouseButtonState[3];

	SceneNodeContainer m_children;

	SpatialGraph m_spatial;
	std::vector<::Camera*> m_cameras;
	std::vector<::Camera*> m_cameraOrder;
	bool m_cameraOrderDirty = true;

	::SceneNode* m_hoverNode = nullptr;
	bool m_canTickHovering = false;

	::SceneNode* m_renderingOrderDirtyNode = nullptr;
	uint32_t m_renderingOrderEnd = 1;
};
