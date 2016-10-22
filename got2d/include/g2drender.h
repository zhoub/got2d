#pragma once

#include <g2dconfig.h>
#include <gmlvector.h>
#include <gmlcolor.h>

namespace g2d
{
	struct GeometryVertex
	{
		gml::vec2 position;
		gml::vec2 texcoord;
		gml::color4 vtxcolor;
	};

	class Mesh
	{
	public:
		~Mesh();
		virtual GeometryVertex* GetRawVertices() = 0;
		virtual unsigned int* GetRawIndices() = 0;
		virtual unsigned int GetVertexCount() = 0;
		virtual unsigned int GetIndexCount() = 0;
		virtual void ResizeVertexArray(unsigned int vertexCount) = 0;
		virtual void ResizeIndexArray(unsigned int indexCount) = 0;
		virtual void Release() = 0;
	};

	class RenderSystem
	{
	public:
		~RenderSystem();
		virtual void BeginRender() = 0;
		virtual void EndRender() = 0;
		virtual Mesh* CreateMesh(unsigned int vertexCount, unsigned int indexCount) = 0;
		virtual void RenderMesh(Mesh*) = 0;
	};
}
