#pragma once
#include "../include/g2dscene.h"
#include <gmlmatrix.h>
#include <g2drender.h>


class Quad : public g2d::Quad
{
	IMPL_CLASSID;
public:
	Quad();
	~Quad();
	virtual void OnInitial() override;
	virtual void OnRender() override;

public:
	inline virtual g2d::Entity* SetSize(const gml::vec2& size) override;
	inline virtual const gml::vec2& GetSize() const override { return m_size; }
	inline virtual void Release() override { delete this; }
	virtual gml::aabb2d GetLocalAABB() const override { return m_aabb; }
	virtual gml::aabb2d GetWorldAABB() const override;

	g2d::Mesh* m_mesh;
	g2d::Material* m_material;
	gml::vec2 m_size;
	gml::aabb2d m_aabb;
};

class Camera : public g2d::Camera
{
	IMPL_CLASSID;
public:
	void SetID(unsigned int index) { m_id = index; }
public:
	virtual void OnUpdate(unsigned int elapsedTime) override;
	virtual void OnUpdateMatrixChanged() override;

public:
	inline virtual void Release() override { delete this; }
	virtual gml::aabb2d GetLocalAABB() const override { return gml::aabb2d(); }
	virtual gml::aabb2d GetWorldAABB() const override { return gml::aabb2d(); }
	inline virtual unsigned int GetID() const override { return m_id; }
	inline virtual g2d::Camera* SetPosition(const gml::vec2& position) override { GetSceneNode()->SetPosition(position); return this; }
	inline virtual g2d::Camera* SetScale(const gml::vec2& scale) override { GetSceneNode()->SetScale(scale); return this; }
	inline virtual g2d::Camera* SetRotation(float radian) override { GetSceneNode()->SetRotation(radian); return this; }
	virtual const gml::mat32& GetViewMatrix() const override { return m_matrix; }
	virtual bool TestVisible(g2d::Entity* entity) override;

private:
	unsigned int m_id;
	gml::mat32 m_matrix;
	gml::aabb2d m_aabb;
};