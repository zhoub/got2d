#pragma once
#include <gml/gmlvector.h>
#include <gml/gmlmatrix.h>
#include <gml/gmlaabb.h>
#include <gml/gmlrect.h>
#include "g2dconfig.h"
#include "g2dinput.h"
namespace g2d
{
	constexpr uint32_t DEF_VISIBLE_MASK = 0xFFFFFFFF;
	constexpr int DEF_COMPONENT_ORDER = 0x5000;

	class Component;
	class Camera;
	class SceneNode;
	class Scene;

	// Component is very most important concept in Got2D engine, 
	// add different components to SceneNode to present varity behaviors.
	// One SceneNode can mounts several componets.
	// Custom component inherit class Component to gain the ability 
	// of acting different behaviors when different events occurred.
	class G2DAPI Component : public GObject
	{
	public:
		// Implement this function to release resources used in the component.
		// Engine will call this function to destroy the component 
		// when a SceneNode no longer being used.
		virtual void Release() = 0;

		// Implement this function to define the boundry of a 
		// visible object, the result of visibility testing depends 
		// on the whether their aabb is intersection with cameras boundtry.
		virtual const gml::aabb2d& GetLocalAABB() const { static gml::aabb2d b; return b; }

		// AABB in world-space, by defaultm, calculating world-space
		// AABB is transforming local AABB with their world transform.
		virtual gml::aabb2d GetWorldAABB() const;

		// Implement this function to redefine the expected execution order.
		// Less number will be executed first.
		virtual int GetExecuteOrder() const { return DEF_COMPONENT_ORDER; }

		// Get the SceneNode holds the components.
		// Provided to the custom component to retrieve the node.
		SceneNode* GetSceneNode() const { return m_sceneNode; }

		// It equals to GetSceneNode()->GetVisibleMask();
		// The mask will collaborate with the camera's visible mask
		uint32_t GetVisibleMask() const;

	public:
		// Trigger when SceneNode being created or the
		// component being added to the node successfully.
		virtual void OnInitial() { }

		// Trigger each frame
		virtual void OnUpdate(uint32_t deltaTime) { }

		// Trigger when the node going to render. 
		// User need to commit a render request to render system.
		virtual void OnRender() { }

		// Trigger when local Roll rotation changes by calling SetRotation().
		virtual void OnRotate(gml::radian r) { }

		// Trigger when local scale changes by calling SetScale().
		virtual void OnScale(const gml::vec2& newScaler) { }

		// Trigger when local position changes by calling SetPosition().
		virtual void OnMove(const gml::vec2& newPos) { }

		// Trigger when raw messages comes. 
		// Some messages will be converted to special events list below.
		virtual void OnMessage(const g2d::Message& message) { }

		// First update event after local tranform changes.
		// It comes after OnUpdate event, do AABB update here.
		virtual void OnUpdateMatrixChanged() { }

		// Trigger when cursor first hit the SceneNode.
		// Adjacent SceneNode is the last cursor hovering node,
		// it would be null if cursor hit the node from empty ground.
		virtual void OnCursorEnterFrom(SceneNode* adjacency, const g2d::Mouse&, const g2d::Keyboard&) { }

		// triggering Repeatly when cursor hovering the node.
		virtual void OnCursorHovering(const g2d::Mouse&, const g2d::Keyboard&) { }

		// Trigger when cursor leave off the SceneNode.
		// Adjacent SceneNode is the next cursor hovering node,
		// it would be null if cursor goes to empty ground.
		virtual void OnCursorLeaveTo(SceneNode* adjacency, const g2d::Mouse&, const g2d::Keyboard&) { }

		// Trigger when node is clicked.
		virtual void OnLClick(const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnRClick(const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnMClick(const g2d::Mouse&, const g2d::Keyboard&) { }

		// Trigger when node is double clicked.
		virtual void OnLDoubleClick(const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnRDoubleClick(const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnMDoubleClick(const g2d::Mouse&, const g2d::Keyboard&) { }

		// Trigger when the node is dragged at first time.
		virtual void OnLDragBegin(const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnRDragBegin(const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnMDragBegin(const g2d::Mouse&, const g2d::Keyboard&) { }

		// Trigger when node is dragging without hitting other nodes.
		virtual void OnLDragging(const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnRDragging(const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnMDragging(const g2d::Mouse&, const g2d::Keyboard&) { }

		// Trigger when node is drag end without hitting other nodes.
		virtual void OnLDragEnd(const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnRDragEnd(const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnMDragEnd(const g2d::Mouse&, const g2d::Keyboard&) { }

		// Trigger when node is dragging with hitting other nodes.
		virtual void OnLDropping(SceneNode* dropped, const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnRDropping(SceneNode* dropped, const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnMDropping(SceneNode* dropped, const g2d::Mouse&, const g2d::Keyboard&) { }

		// Trigger when node is drag end with hitting other nodes.
		virtual void OnLDropTo(SceneNode* dropped, const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnRDropTo(SceneNode* dropped, const g2d::Mouse&, const g2d::Keyboard&) { }
		virtual void OnMDropTo(SceneNode* dropped, const g2d::Mouse&, const g2d::Keyboard&) { }

		// Trigger when key is pressed, mutex to pressing.
		virtual void OnKeyPress(KeyCode key, const g2d::Mouse&, const g2d::Keyboard& keyboard) { }

		// Trigger when key is pressing at first time, mutex to pressed.
		virtual void OnKeyPressingBegin(KeyCode key, const g2d::Mouse&, const g2d::Keyboard& keyboard) { }

		// Trigger when key pressing repeatly. 
		virtual void OnKeyPressing(KeyCode key, const g2d::Mouse&, const g2d::Keyboard& keyboard) { }

		// Trigger when the key is released.
		virtual void OnKeyPressingEnd(KeyCode key, const g2d::Mouse&, const g2d::Keyboard& keyboard) { }

	public:// internal use
		void SetSceneNode(g2d::SceneNode* node);

		void SetRenderingOrder(uint32_t& order);

		uint32_t GetRenderingOrder() { return m_renderingOrder; }

	private:
		SceneNode* m_sceneNode = nullptr;
		uint32_t m_renderingOrder = 0xFFFFFFFF;
	};

	// Image Quad
	class G2DAPI Quad : public Component
	{
	public:
		static Quad* Create();

		// Resize mesh size of the Quad.
		virtual g2d::Quad* SetSize(const gml::vec2& size) = 0;

		// Get the mesh size of the Quad.
		virtual const gml::vec2& GetSize() const = 0;
	};

	// Camera is used for visibility testing & rendering
	class G2DAPI Camera : public Component
	{
	public:
		// Index in scene, index of main(default) camera is 0
		virtual uint32_t GetID() const = 0;

		// Setting eye's position.
		virtual Camera* SetPosition(const gml::vec2& p) = 0;

		// Setting camera's boundry.
		virtual Camera* SetScale(const gml::vec2& s) = 0;

		// Setting camera's Roll rotation.
		virtual Camera* SetRotation(gml::radian r) = 0;

		// Setting camera's rendering order.
		// Multiple cameras need to be sort before rendering,
		// less rendering order will be rendered first
		virtual void SetRenderingOrder(int order) = 0;

		// Visible matching mask, used for visibility testing.
		// If scene nodes's mask is overlapped this mask,
		// we treat it as visible.
		virtual void SetVisibleMask(uint32_t mask) = 0;

		// Wont do the visibility testing and rendering if camera is not activated.
		virtual void SetActivity(bool activity) = 0;

		virtual const gml::mat32& GetViewMatrix() const = 0;

		// Check whether an AABB is intersect with the camera boundry.
		// Point AABB is treat as not visible.
		virtual bool TestVisible(const gml::aabb2d& bounding) const = 0;

		// Check whether a component is visible.
		// Point AABB, mask is not matching, will be regard to not visible.
		virtual bool TestVisible(Component& component) const = 0;

		virtual uint32_t GetVisibleMask() const = 0;

		virtual int GetRenderingOrder() const = 0;

		// Is camera is not activated ? 
		virtual bool IsActivity() const = 0;

		// Convert screen-space coordinate to world-space coordinate
		virtual gml::vec2 ScreenToWorld(const gml::coord& pos) const = 0;

		// Convert world-space coordinate to screen-space coordinate
		virtual gml::coord WorldToScreen(const gml::vec2& pos) const = 0;
	};

	// Node in Scene Tree
	class G2DAPI SceneNode : public GObject
	{
	public:
		// Destroy the node, remove from its parent, and release all components.
		virtual void Release() = 0;

		// Which scene the node belongs to.
		virtual Scene* GetScene() const = 0;

		// Return nullptr if the parent is Scene.
		virtual SceneNode* GetParentNode() const = 0;

		// Return nullptr if the node is the first child.
		virtual SceneNode* GetPrevSiblingNode() const = 0;

		// Return nullptr if the node is the last child.
		virtual SceneNode* GetNextSiblingNode() const = 0;

		// Retrieve the first child.
		// If scene have no children, return nullptr.
		virtual g2d::SceneNode* FirstChild() const = 0;

		// Retrieve the last child.
		// If scene have no children,  return nullptr.
		virtual g2d::SceneNode* LastChild() const = 0;

		virtual g2d::SceneNode* GetChildByIndex(uint32_t index) const = 0;

		virtual uint32_t GetChildCount() const = 0;

		// Create a child node, and add it to the node
		virtual SceneNode* CreateChild() = 0;

		// Swap index with the LAST child, make sure the 
		// node is to rendering at bottom.
		// CAUTION: Do nothing when the node is the first.
		virtual void MoveToFront() = 0;

		// Swap index with the FIRST child, make sure the 
		// node is to rendering at top.
		// CAUTION: Do nothing when the node is the last.
		virtual void MoveToBack() = 0;

		// Swap index with PREV child, to adjust rendering order.
		// Do nothing when the node is the first child.
		virtual void MovePrev() = 0;

		// Swap index with NEXT child, to adjust rendering order.
		// Do nothing when the node is the last child.
		virtual void MoveNext() = 0;

		virtual bool AddComponent(Component*, bool autoRelease) = 0;

		// Remove a component with auto release setting.
		// Component::Release will be called if auto release is set.
		virtual bool RemoveComponent(Component*) = 0;

		// Remove a component without releasing.
		virtual bool RemoveComponentWithoutRelease(Component*) = 0;

		// Query auto release state of a certain component.
		// Return false if component is removed or not exist.
		virtual bool IsComponentAutoRelease(Component*) const = 0;

		// Get component by index.
		// CAUTION: Index of component will changes 
		// due to change the execution order.
		virtual Component* GetComponentByIndex(uint32_t index) const = 0;

		virtual uint32_t GetComponentCount() const = 0;

		virtual const gml::mat32& GetLocalMatrix() = 0;

		virtual const gml::mat32& GetWorldMatrix() = 0;

		// Setting local-space position.
		virtual SceneNode* SetPosition(const gml::vec2& position) = 0;

		// Setting world-space position, it will changes local position.
		virtual g2d::SceneNode* SetWorldPosition(const gml::vec2& position) = 0;

		// Setting world-space right direction, it will changes local rotation.
		virtual g2d::SceneNode* SetRight(const gml::vec2& right) = 0;

		// Setting world-space up direction, it will changes local rotation.
		virtual g2d::SceneNode* SetUp(const gml::vec2& up) = 0;

		// Setting offset of center.
		virtual SceneNode* SetPivot(const gml::vec2& pivot) = 0;

		// Setting local scale.
		virtual SceneNode* SetScale(const gml::vec2& scale) = 0;

		// Setting local Roll rotation.
		virtual SceneNode* SetRotation(gml::radian r) = 0;

		// Manually setting visibility.
		// None-seen node will not be rendered.
		virtual void SetVisible(bool) = 0;

		// Static node will be add to spatial graph(quadtree)
		// to boost efficiency of visibility testing.
		virtual void SetStatic(bool) = 0;

		// Setting visible mask.
		// The mask will collaborate with camera's visible mask 
		// to determin whether it will be seen by the camera.
		virtual void SetVisibleMask(uint32_t mask, bool recursive) = 0;

		// Local position.
		virtual const gml::vec2& GetPosition() const = 0;

		// World-space position, it equals to local position tranfromed by world transform.
		virtual gml::vec2 GetWorldPosition() = 0;

		// World-space right direction.
		virtual const gml::vec2& GetRight() = 0;

		// World-space up directoin.
		virtual const gml::vec2& GetUp() = 0;

		// Local offset of center.
		virtual const gml::vec2& GetPivot() const = 0;

		// Local scale
		virtual const gml::vec2& GetScale() const = 0;

		// Local Roll rotation 
		virtual gml::radian GetRotation() const = 0;

		// Query the child index of his father.
		virtual uint32_t GetChildIndex() const = 0;

		virtual bool IsVisible() const = 0;

		virtual bool IsStatic() const = 0;

		// Visible Mask will collaborate with camera's visible mask 
		// to determin whether the node will be seen by the camera.
		virtual uint32_t GetVisibleMask() const = 0;

		// Convert a world-space coordinate to the local-space coordinate.
		virtual gml::vec2 WorldToLocal(const gml::vec2& pos) = 0;

		// Convert a world-space coordinate to the parent-space coordinate.
		virtual gml::vec2 WorldToParent(const gml::vec2& pos) = 0;
	};

	// Query the first component by a certain type.
	template<typename T> T* FindComponent(SceneNode* node);

	class G2DAPI Scene : public GObject
	{
	public:
		// Call this manually when the scene no longer 
		// being used, to destroy the entire scene tree
		// and itself.
		// Function can ONLY be called ONCE.
		virtual void Release() = 0;

		// Retrieve the first child.
		// If scene have no children, return nullptr.
		virtual g2d::SceneNode* FirstChild() const = 0;

		// Retrieve the last child.
		// If scene have no children, return nullptr.
		virtual g2d::SceneNode* LastChild() const = 0;

		virtual g2d::SceneNode* GetChildByIndex(uint32_t index) const = 0;

		virtual uint32_t GetChildCount() const = 0;

		// Create a child node, and add it to the node.
		virtual SceneNode* CreateChild() = 0;

		// Create a child node, with a camera component, 
		// and add it the scene.
		virtual Camera* CreateCameraNode() = 0;

		// Default camera, it return the result of 
		// scene->GetCameraByIndex(0);
		virtual Camera* GetMainCamera() const = 0;
		
		// Get camera by ID.
		// The ID of main(default) camera is 0.
		virtual Camera* GetCameraByIndex(uint32_t index) const = 0;

		// Number of exist cameras.
		virtual uint32_t GetCameraCount() const = 0;
		
		// Call it manually each frame, to send OnRender event
		// to scene tree, so that each node can notify its components.
		virtual void Render() = 0;
	};

	template<typename T> T* FindComponent(SceneNode* node)
	{
		auto count = node->GetComponentCount();
		for (uint32_t i = 0; i < count; i++)
		{
			auto component = node->GetComponentByIndex(i);
			if (Is<T>(component)) return reinterpret_cast<T*>(component);
		}
		return nullptr;
	}
}
