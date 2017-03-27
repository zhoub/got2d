#pragma once
#include "../include/g2dscene.h"
#include "component.h"
#include "spatial_graph.h"
#include "input.h"
#include <gmlmatrix.h>
#include <vector>

class SceneNode;
class Scene;

// for collection
// in order to not to do the converting
struct NodeComponent
{
	NodeComponent(g2d::Component* c, bool ar) : ComponentPtr(c), AutoRelease(ar) { }
	g2d::Component* ComponentPtr = nullptr;
	bool AutoRelease = false;
};

class BaseNode
{
public:
	virtual uint32_t _GetRenderingOrder() const = 0;

	virtual void AdjustRenderingOrder() = 0;

public:
	void _SetVisibleMask(uint32_t mask, bool recursive);

	void _SetPosition(const gml::vec2& position);

	void _SetPivot(const gml::vec2& pivot);

	void _SetScale(const gml::vec2& scale);

	void _SetRotation(gml::radian r);

	void _SetVisible(bool visible) { m_isVisible = visible; }

	::SceneNode* _FirstChild() const;

	::SceneNode* _LastChild() const;

	::SceneNode* _GetChildByIndex(uint32_t index) const;

	uint32_t _GetChildCount() const { return static_cast<uint32_t>(m_children.size()); }

	void RemoveChildNode(::SceneNode* child);

	void MoveChild(uint32_t from, uint32_t to);

	bool _AddComponent(g2d::Component* component, bool autoRelease, g2d::SceneNode* node);

	bool _RemoveComponent(g2d::Component* component, bool forceNotReleased);

	bool _IsComponentAutoRelease(g2d::Component* component) const;

	g2d::Component* _GetComponentByIndex(uint32_t index) const;

	uint32_t _GetComponentCount() const { return static_cast<uint32_t>(m_components.size()); }

	const gml::mat32& _GetLocalMatrix();

	const gml::vec2& _GetPosition() const { return m_position; }

	const gml::vec2& _GetPivot() const { return m_pivot; }

	const gml::vec2& _GetScale() const { return m_scale; }

	gml::radian _GetRotation() const { return m_rotation; }

	uint32_t _GetVisibleMask() const { return m_visibleMask; }

	bool _IsVisible() const { return m_isVisible; }

public: // traversal 
	template<typename FUNC> void TraversalChildren(FUNC func)
	{
		for (auto& child : m_children) func(child);
	}

	template<typename FUNC> void InverseTraversalChildren(FUNC func)
	{
		auto it = std::rbegin(m_children);
		auto end = std::rend(m_children);
		for (; it != end; it++) func(*child);
	}

	template<typename FUNC> void TraversalComponent(FUNC f)
	{
		for (auto& c : m_components) f(c.ComponentPtr);
	}

protected:
	BaseNode();

	BaseNode(const BaseNode&) = delete;

	~BaseNode();

	::SceneNode* _CreateSceneNodeChild(::Scene& scene, ::SceneNode* parent);

	::SceneNode* _CreateSceneNodeChild(::Scene& scene);

	void OnUpdateChildren(uint32_t deltaTime);

	void EmptyChildren();

	void OnMessageComponentsAndChildren(const g2d::Message& message);

	void OnKeyPressComponentsAndChildren(g2d::KeyCode key);

	void OnKeyPressingBeginComponentsAndChildren(g2d::KeyCode key);

	void OnKeyPressingComponentsAndChildren(g2d::KeyCode key);

	void OnKeyPressingEndComponentsAndChildren(g2d::KeyCode key);

	template<typename CFUNC>
	void DispatchComponent(const CFUNC& cf)
	{
		CollectComponents();
		for (auto& c : m_collectionComponents)
		{
			cf(c.ComponentPtr);
		}
		DelayRemoveComponents();
	}

	template<typename NFUNC>
	void DispatchChildren(const NFUNC& nf)
	{
		// 不能先collect，有可能产生添加递归的潜在问题
		for (auto& child : m_collectionChildren)
		{
			nf(child);
		};
		CollectChildren();
	}

	template<typename CFUNC, typename NFUNC>
	void DispatchRecursive(const CFUNC& cf, const NFUNC& nf)
	{
		DispatchComponent(cf);
		DispatchChildren(nf);
	}

	void SetLocalMatrixDirty();

private:
	void RecollectChildren();

	void RecollectComponents();

	void DelayRemoveChildren();

	void DelayRemoveComponents();

	void CollectComponents();

	void CollectChildren();

	gml::vec2 m_position;
	gml::vec2 m_pivot;
	gml::vec2 m_scale;
	gml::radian m_rotation;
	gml::mat32 m_matrixLocal;
	bool m_matrixLocalDirty = true;
	bool m_isVisible = true;
	uint32_t m_visibleMask = g2d::DEF_VISIBLE_MASK;

	std::vector<::SceneNode*> m_children;
	std::vector<::SceneNode*> m_releasedChildren;
	std::vector<::SceneNode*> m_collectionChildren;
	bool m_childrenChanged = true;

	std::vector<NodeComponent> m_components;
	std::vector<NodeComponent> m_releasedComponents;
	std::vector<NodeComponent> m_collectionComponents;
	bool m_componentsChanged = true;
};

class SceneNode : public g2d::SceneNode, public BaseNode
{
	RTTI_IMPL;
public:
	SceneNode(::Scene& scene, ::SceneNode* parent, uint32_t childID);

	SceneNode(::Scene& parent, uint32_t childID);

	~SceneNode();

	void OnUpdate(uint32_t deltaTime);

	void SetChildIndex(uint32_t index) { m_childIndex = index; }

	void SetRenderingOrder(uint32_t& order);

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
	virtual g2d::Scene* GetScene() const override;

	virtual g2d::SceneNode* GetParentNode() const override { return &m_iparent; }

	virtual g2d::SceneNode* GetPrevSiblingNode() const override { return GetPrevSibling(); }

	virtual g2d::SceneNode* GetNextSiblingNode() const override { return GetNextSibling(); }

	virtual g2d::SceneNode* FirstChild() const override { return _FirstChild(); }

	virtual g2d::SceneNode* LastChild() const override { return _LastChild(); }

	virtual g2d::SceneNode* GetChildByIndex(uint32_t index) const override { return _GetChildByIndex(index); }

	virtual uint32_t GetChildCount() const override { return _GetChildCount(); }

	virtual g2d::SceneNode* CreateChild() override;

	virtual void Remove() override { m_bparent.RemoveChildNode(this); }

	virtual void MoveToFront() override;

	virtual void MoveToBack() override;

	virtual void MovePrev() override;

	virtual void MoveNext() override;

	virtual bool AddComponent(g2d::Component* component, bool autoRelease) override { return _AddComponent(component, autoRelease, this); }

	virtual bool RemoveComponent(g2d::Component* component) override { return _RemoveComponent(component, false); }

	virtual bool RemoveComponentWithoutReleased(g2d::Component* component) override { return _RemoveComponent(component, true); }

	virtual bool IsComponentAutoRelease(g2d::Component* component) const override { return _IsComponentAutoRelease(component); }

	virtual g2d::Component* GetComponentByIndex(uint32_t index) const override { return _GetComponentByIndex(index); }

	virtual uint32_t GetComponentCount() const override { return _GetComponentCount(); }

	virtual const gml::mat32& GetLocalMatrix() override { return _GetLocalMatrix(); }

	virtual const gml::mat32& GetWorldMatrix() override;

	virtual g2d::SceneNode* SetPosition(const gml::vec2& position) override;

	virtual g2d::SceneNode* SetPivot(const gml::vec2& pivot) override;

	virtual g2d::SceneNode* SetScale(const gml::vec2& scale) override;

	virtual g2d::SceneNode* SetRotation(gml::radian r) override;

	virtual void SetVisible(bool visible) override { _SetVisible(visible); }

	virtual void SetStatic(bool s) override;

	virtual void SetVisibleMask(uint32_t mask, bool recursive) override { _SetVisibleMask(mask, recursive); }

	virtual const gml::vec2& GetPosition()  const override { return _GetPosition(); }

	virtual const gml::vec2& GetPivot() const override { return _GetPivot(); }

	virtual const gml::vec2& GetScale() const override { return _GetScale(); }

	virtual gml::radian GetRotation() const override { return _GetRotation(); }

	virtual gml::vec2 GetWorldPosition() override;

	virtual uint32_t GetChildIndex() const override { return m_childIndex; }

	virtual bool IsVisible() const override { return _IsVisible(); }

	virtual bool IsStatic() const override { return m_isStatic; }

	virtual uint32_t GetVisibleMask() const override { return _GetVisibleMask(); }

	virtual gml::vec2 WorldToLocal(const gml::vec2& pos) override;

	virtual gml::vec2 WorldToParent(const gml::vec2& pos) override;

	::SceneNode* GetPrevSibling() const;

	::SceneNode* GetNextSibling() const;

	// 这个是有可能返回空节点的，当为空的时候，父亲为m_scene
	::SceneNode* GetParent() const { return m_parent; }

public:	// BaseNode
	virtual uint32_t _GetRenderingOrder() const override { return m_renderingOrder; }

	virtual void AdjustRenderingOrder() override;


private:
	void SetWorldMatrixDirty();

	void AdjustSpatial();

	uint32_t m_childIndex = 0;
	uint32_t m_renderingOrder = 0xFFFFFFFF;//保证一开始是错误的
	::Scene& m_scene;
	::BaseNode& m_bparent;
	g2d::SceneNode& m_iparent;
	::SceneNode* m_parent = nullptr;
	bool m_isStatic = false;
	bool m_matrixDirtyEntityUpdate = true;
	bool m_matrixWorldDirty = true;
	gml::mat32 m_matrixWorld;
};

class Scene : public g2d::Scene, public BaseNode
{
	RTTI_IMPL;
public:
	Scene(float boundSize);

	void SetCameraOrderDirty() { m_cameraOrderDirty = true; }

	SpatialGraph* GetSpatialGraph() { return &m_spatial; }

	void Update(uint32_t elapsedTime, uint32_t deltaTime);

	void OnMessage(const g2d::Message& message, uint32_t currentTimeStamp);

	void SetRenderingOrderDirty(BaseNode* parent);

public: //g2d::SceneNode
	virtual g2d::Scene* GetScene() const override { return const_cast<::Scene*>(this); }

	virtual SceneNode* GetParentNode() const override { return nullptr; }

	virtual SceneNode* GetPrevSiblingNode() const override { return nullptr; }

	virtual SceneNode* GetNextSiblingNode() const override { return nullptr; }

	virtual g2d::SceneNode* FirstChild() const override { return _FirstChild(); }

	virtual g2d::SceneNode* LastChild() const override { return _LastChild(); }

	virtual g2d::SceneNode* GetChildByIndex(uint32_t index) const override { return _GetChildByIndex(index); }

	virtual uint32_t GetChildCount() const override { return _GetChildCount(); }

	virtual g2d::SceneNode* CreateChild() override;

	virtual void Remove() override { }

	virtual void MoveToFront() override { }

	virtual void MoveToBack() override { }

	virtual void MovePrev() override { }

	virtual void MoveNext() override { }

	virtual bool AddComponent(g2d::Component* component, bool autoRelease) override { return _AddComponent(component, autoRelease, this); }

	virtual bool RemoveComponent(g2d::Component* component) override { return _RemoveComponent(component, false); }

	virtual bool RemoveComponentWithoutReleased(g2d::Component* component) override { return _RemoveComponent(component, true); }

	virtual bool IsComponentAutoRelease(g2d::Component* component) const override { return _IsComponentAutoRelease(component); }

	virtual g2d::Component* GetComponentByIndex(uint32_t index) const override { return _GetComponentByIndex(index); }

	virtual uint32_t GetComponentCount() const override { return _GetComponentCount(); }

	virtual const gml::mat32& GetLocalMatrix() override { return _GetLocalMatrix(); }

	virtual const gml::mat32& GetWorldMatrix() override { return _GetLocalMatrix(); }

	virtual void SetVisibleMask(uint32_t mask, bool recursive) override { _SetVisibleMask(mask, recursive); }

	virtual g2d::SceneNode* SetPosition(const gml::vec2& position) override { _SetPosition(position); return this; }

	virtual g2d::SceneNode* SetPivot(const gml::vec2& pivot) override { _SetPivot(pivot); return this; }

	virtual g2d::SceneNode* SetScale(const gml::vec2& scale) override { _SetScale(scale); return this; }

	virtual g2d::SceneNode* SetRotation(gml::radian r) override { _SetRotation(r); return this; }

	virtual void SetVisible(bool visible) override { _SetVisible(visible); }

	virtual void SetStatic(bool visible) override { }

	virtual const gml::vec2& GetPosition()  const override { return _GetPosition(); }

	virtual const gml::vec2& GetPivot() const override { return _GetPivot(); }

	virtual const gml::vec2& GetScale() const override { return _GetScale(); }

	virtual gml::radian GetRotation() const override { return _GetRotation(); }

	virtual gml::vec2 GetWorldPosition() override { return _GetPosition(); };

	virtual uint32_t GetChildIndex() const override { return 0; }

	virtual bool IsVisible() const override { return _IsVisible(); }

	virtual bool IsStatic() const override { return true; }

	virtual uint32_t GetVisibleMask() const override { return _GetVisibleMask(); }

	virtual gml::vec2 WorldToLocal(const gml::vec2& pos) override { return pos; }

	virtual gml::vec2 WorldToParent(const gml::vec2& pos) override { return pos; }

public:	//g2d::Scene
	virtual void Release() override;

	virtual g2d::Camera* CreateCameraNode() override;

	virtual g2d::Camera* GetMainCamera() const override { return GetCameraByIndex(0); }

	virtual g2d::Camera* GetCameraByIndex(uint32_t) const override;

	virtual void Render() override;

public:	// BaseNode
	virtual uint32_t _GetRenderingOrder() const override { return 0; }

	virtual void AdjustRenderingOrder() override;

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

	SpatialGraph m_spatial;
	std::vector<::Camera*> m_cameras;
	std::vector<::Camera*> m_cameraOrder;
	bool m_cameraOrderDirty = true;

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

	::SceneNode* m_hoverNode = nullptr;
	bool m_canTickHovering = false;
	BaseNode* m_renderingOrderDirtyNode = nullptr;
	uint32_t m_renderingOrderEnd = 1;
};
