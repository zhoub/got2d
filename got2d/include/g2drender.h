#pragma once

#include <g2dconfig.h>
#include <gmlvector.h>
#include <gmlmatrix.h>
#include <gmlcolor.h>

namespace g2d
{
	enum BlendMode
	{
		BLEND_NONE,
		BLEND_NORMAL,
		BLEND_ADD,
		BLEND_SCREEN,
	};

	struct GeometryVertex
	{
		gml::vec2 position;
		gml::vec2 texcoord;
		gml::color4 vtxcolor;
	};

	class Mesh
	{
	public:
		virtual ~Mesh();
		virtual GeometryVertex* GetRawVertices() = 0;
		virtual unsigned int* GetRawIndices() = 0;
		virtual unsigned int GetVertexCount() = 0;
		virtual unsigned int GetIndexCount() = 0;
		virtual void ResizeVertexArray(unsigned int vertexCount) = 0;
		virtual void ResizeIndexArray(unsigned int indexCount) = 0;
		virtual void Release() = 0;
	};

	class Texture
	{
	public:
		virtual ~Texture();
		virtual bool IsSame(Texture* other) const = 0;
		virtual void AddRef() = 0;
		virtual void Release() = 0;
	};

	//keep it simple and stupid.
	//there is no other more easy way to define param setting interface elegantly
	//TODO: make it more elegant.
	class Pass
	{
	public:
		virtual ~Pass();
		virtual const char* GetVertexShaderName() const = 0;
		virtual const char* GetPixelShaderName() const = 0;
		virtual bool IsSame(Pass* other) const = 0;
		virtual void SetTexture(unsigned int index, Texture*, bool autoRelease) = 0;
		virtual void SetVSConstant(unsigned int index, float* data, unsigned int size, unsigned int count) = 0;
		virtual void SetPSConstant(unsigned int index, float* data, unsigned int size, unsigned int count) = 0;
		virtual void SetBlendMode(BlendMode) = 0;
		virtual Texture* GetTexture(unsigned int index) const = 0;
		virtual unsigned int GetTextureCount() const = 0;
		virtual const float* GetVSConstant() const = 0;
		virtual unsigned int GetVSConstantLength() const = 0;
		virtual const float* GetPSConstant() const = 0;
		virtual unsigned int GetPSConstantLength() const = 0;
		virtual BlendMode GetBlendMode() const = 0;
		virtual void Release() = 0;
	};

	class Material
	{
	public:
		virtual ~Material();
		virtual Pass* GetPass(unsigned int index) const = 0;
		virtual unsigned int GetPassCount() const = 0;
		virtual bool IsSame(Material* other) const = 0;
		virtual Material* Clone() const = 0;
		virtual void Release() = 0;
	};

	enum RenderOrder
	{
		RORDER_PREZ = 0,
		RORDER_BACKGROUND = 0x4000,
		RORDER_DEFAULT = RORDER_BACKGROUND + 0x1000,
		RORDER_FOREGROUND = RORDER_BACKGROUND + 0x400,
		RORDER_OVERLAY = 0x8000,
	};
	class RenderSystem
	{
	public:
		virtual ~RenderSystem();
		virtual bool OnResize(long width, long height) = 0;
		virtual void BeginRender() = 0;
		virtual void EndRender() = 0;
		virtual void RenderMesh(unsigned int layer, Mesh*, Material*, const gml::mat32&) = 0;
		virtual long GetWindowWidth() const = 0;
		virtual long GetWindowHeight() const = 0;
	public:
		virtual Mesh* CreateMesh(unsigned int vertexCount, unsigned int indexCount) = 0;
		virtual Material* CreateColorTextureMaterial() = 0;
		virtual Material* CreateSimpleTextureMaterial() = 0;
		virtual Material* CreateSimpleColorMaterial() = 0;
	};
}
