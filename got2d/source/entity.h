#pragma once
#include "../include/g2dscene.h"
#include "inner_utility.h"
#include "scope_utility.h"
#include <gmlmatrix.h>
#include <g2drender.h>
#include <vector>


class Quad : public g2d::Quad
{
	RTTI_IMPL;
public:
	Quad();

public:	//g2d::Entity
	virtual void Release() override { delete this; }

	virtual const gml::aabb2d& GetLocalAABB() const override { return m_aabb; }

	virtual void OnInitial() override;

	virtual void OnRender() override;

public:	//g2d::Quad
	virtual g2d::Quad* SetSize(const gml::vec2& size) override;

	virtual const gml::vec2& GetSize() const override { return m_size; }

	autor<g2d::Mesh> m_mesh = nullptr;
	autor<g2d::Material> m_material = nullptr;
	gml::vec2 m_size;
	gml::aabb2d m_aabb;
};

class Camera : public g2d::Camera
{
	RTTI_IMPL;
public:
	void SetID(uint32_t index) { m_id = index; }

	g2d::Entity* FindIntersectionObject(const gml::vec2& worldPosition);

	std::vector<Entity*> visibleEntities;

public:	//g2d::entity
	virtual void Release() override { delete this; }

	virtual const gml::aabb2d& GetLocalAABB() const override { return m_aabb; }

	virtual void OnUpdate(uint32_t elapsedTime) override;

	virtual void OnUpdateMatrixChanged() override;

public:	//g2d::camera
	virtual uint32_t GetID() const override { return m_id; }

	virtual g2d::Camera* SetPosition(const gml::vec2& position) override { GetSceneNode()->SetPosition(position); return this; }

	virtual g2d::Camera* SetScale(const gml::vec2& scale) override { GetSceneNode()->SetScale(scale); return this; }

	virtual g2d::Camera* SetRotation(gml::radian r) override { GetSceneNode()->SetRotation(r); return this; }

	virtual void SetRenderingOrder(int renderingOrder) override;

	virtual void SetVisibleMask(uint32_t mask) override { m_visibleMask = mask; }

	virtual void SetActivity(bool activity) override { m_activity = activity; }

	virtual const gml::mat32& GetViewMatrix() const override { return m_matView; }

	virtual bool TestVisible(const gml::aabb2d& bounding) const override;

	virtual bool TestVisible(g2d::Entity& entity) const override;

	virtual uint32_t GetVisibleMask() const override { return m_visibleMask; }

	virtual int GetRenderingOrder() const override { return m_renderingOrder; }

	virtual bool IsActivity() const override { return m_activity; }
	
	virtual gml::vec2 ScreenToWorld(const gml::coord& pos) const override;

	virtual gml::coord WorldToScreen(const gml::vec2 & pos) const override;

private:
	uint32_t m_id;
	uint32_t m_visibleMask = g2d::DEFAULT_VISIBLE_MASK;
	int m_renderingOrder = 0;
	bool m_activity = true;
	gml::mat32 m_matView;
	gml::mat33 m_matViewInverse;
	gml::aabb2d m_aabb;
};
