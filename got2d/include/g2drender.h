#pragma once
#include <gml/gmlvector.h>
#include <gml/gmlmatrix.h>
#include <gml/gmlcolor.h>
#include <gml/gmlrect.h>
#include "g2dconfig.h"

namespace g2d
{
	// Semi-transparent blending method.
	enum class G2DAPI BlendMode
	{
		None,		// src*1 + dst*0 = src

		Normal,		// src*src_a + dst*(1-src_a)

		Additve,	// src*1 + dst*1
	};

	// Memory layout of Mesh.
	struct GeometryVertex
	{
		gml::vec2 position;
		gml::vec2 texcoord;
		gml::color4 vtxcolor;
	};

	// User defined model mesh, it is a render resource.
	// Mesh data save in memory, render system will upload 
	// datas to video memory when rendering, depends on which
	// material be used.
	class G2DAPI Mesh : public GObject
	{
	public:
		static Mesh* Create(uint32_t vertexCount, uint32_t indexCount);

		// Call this manually when the mesh no longer 
		// being used, to release used memory and itself.
		// Function can ONLY be called ONCE.
		virtual void Release() = 0;

		// Retrieve the memory pointer of vertices data.
		// User write datas to the adress pointer to fill 
		// mesh vertices. Make sure not to exceed the boundry 
		// of the vertex data memory, otherwise it will 
		// raise a out-of-memory exception.
		// length = sizeof(GeometryVertex) * VertexCount;
		virtual const GeometryVertex* GetRawVertices() const = 0;
		virtual GeometryVertex* GetRawVertices() = 0;

		// Retrieve the memory pointer of indices data.
		// User write datas to the adress pointer to fill 
		// mesh indices. Make sure not to exceed the boundry 
		// of the index data memory, otherwise it will 
		// raise a out-of-memory exception.
		// length = sizeof(uint32_t) * IndexCount.
		virtual const uint32_t* GetRawIndices() const = 0;
		virtual uint32_t* GetRawIndices() = 0;

		// Number of vertices.
		// length = sizeof(GeometryVertex) * VertexCount;
		virtual uint32_t GetVertexCount() const = 0;

		// Number of indices.
		// length = sizeof(uint32_t) * IndexCount;
		virtual uint32_t GetIndexCount() const = 0;

		// Resize the length of vertex data memory, old data will be keeped.
		virtual void ResizeVertexArray(uint32_t vertexCount) = 0;

		// Resize the length of index data memory, old data will be keeped.
		virtual void ResizeIndexArray(uint32_t indexCount) = 0;

		// Merge another mesh to the mesh, with a given transform.
		// The transfrom will apply to that mesh, regard it as world transform
		virtual bool Merge(Mesh* other, const gml::mat32& transform) = 0;
	};

	// Texture is a render resource refer to 2d image.
	// It is a shared resources, so manually calling AddRef to increasing
	// reference count, and do Release when it no longer being used.
	class G2DAPI Texture : public GObject
	{
	public:
		// Read and parse colors from a image file, it support 
		// BMP/PNG/TGA files, with or without alpha channel.
		// Return nullptr if meets an unsupport file format,
		// or an error occured when loading.
		static Texture* LoadFromFile(const char* path);
		
		// Call this manually when the texture no longer
		// being referenced, to decrease reference count.
		// Texture will be destroyed when count drop to 0.
		virtual void Release() = 0;

		// Call this manually when texture being referenced
		// to add reference count.
		virtual void AddRef() = 0;

		// Texture ID.
		// At this stage, texture can only be create by loading 
		// from files, so ID eaquals to file path of the file.
		virtual const char* Identifier() const = 0;

		// Return true if texture is loading from same file,
		// otherwise return false.
		virtual bool IsSame(Texture* other) const = 0;
	};

	// Pass is a rendering resouces, a pass refer to one drawcall.
	/// Keep it simple and stupid.
	/// there is no other more easy way to define param setting interface elegantly
	/// TODO: make it more elegant.
	class G2DAPI Pass : public GObject
	{
	public:
		// Indicate what vertex shader will be used.
		virtual const char* GetVertexShaderName() const = 0;

		// Indicate what pixel shader will be used.
		virtual const char* GetPixelShaderName() const = 0;

		// Return true if two passes using same VS/PS, and
		// the datas including in passes is same. otherwise return false.
		virtual bool IsSame(Pass* other) const = 0;

		virtual void SetBlendMode(BlendMode mode) = 0;

		// Fill datas to the vertex constant buffer, user should 
		// make sure not to exceed the boundry of the buffer memory,
		// or it will raise a out-of-memory exception.
		virtual void SetVSConstant(uint32_t index, float* data, uint32_t size, uint32_t count) = 0;

		// Fill datas to the pixel constant buffer, user should 
		// make sure not to exceed the boundry of the buffer memory,
		// or it will raise a out-of-memory exception.
		virtual void SetPSConstant(uint32_t index, float* data, uint32_t size, uint32_t count) = 0;

		// Set a texture to a pass.
		// if texture index is exceed the max texture index,
		// it will automatic expand the max texture. count of Texture
		// will changes, and those index between old/new max index, 
		// will be set to nullptr.
		// Render system will keep last sampling states of each slot when 
		// there is no texture in the slot during render process.
		virtual void SetTexture(uint32_t index, Texture*, bool autoRelease) = 0;

		// Not every slot has solid data, it means some index
		// may return nullptr when they never be set.
		virtual Texture* GetTextureByIndex(uint32_t index) const = 0;

		// Get the max count of texture slot.
		virtual uint32_t GetTextureCount() const = 0;

		// Retrieve vertex constant buffer pointer.
		virtual const float* GetVSConstant() const = 0;

		// memory size of vertex constant buffer.
		virtual uint32_t GetVSConstantLength() const = 0;

		// Retrieve pixel constant buffer pointer.
		virtual const float* GetPSConstant() const = 0;

		// memory size of pixel constant buffer.
		virtual uint32_t GetPSConstantLength() const = 0;

		virtual BlendMode GetBlendMode() const = 0;
	};

	// Material is another rendering resources.
	// One material can holds one or more passes,
	// each pass will save the material info datas,
	// so do NOT use one material with different objects.
	class G2DAPI Material : public GObject
	{
	public:
		// Create a new material that rendering with 
		// combination of vertex color and texture color
		static Material* CreateColorTexture();

		// Create a new material that rendering with texture color
		static Material* CreateSimpleTexture();

		// Create a new material that rendering with vertex color 
		static Material* CreateSimpleColor();

		// Call this manually when the material no longer 
		// being used, to release passes and itself.
		// Function can ONLY be called ONCE.
		virtual void Release() = 0;

		// Usually, a material holds one pass, but some can 
		// holds multiple passes, such as filtering,
		// silhouette materials, they can invoke multiple drawcalls.
		virtual Pass* GetPassByIndex(uint32_t index) const = 0;

		virtual uint32_t GetPassCount() const = 0;

		// Check whether two material holds same passes.
		virtual bool IsSame(Material* other) const = 0;

		// Create a new material contains same passes inside.
		virtual Material* Clone() const = 0;
	};

	// Define the order of a render request.
	// In 2D rendering, adjusting rendering order is a common need.
	// e.g. we need draw shadow sprites before all character sprites
	class G2DAPI RenderLayer
	{
	public:
		constexpr static uint32_t PreZ = 0x1000;
		constexpr static uint32_t BackGround = 0x2000;
		constexpr static uint32_t Default = BackGround + 0x2000;
		constexpr static uint32_t ForeGround = BackGround + 0x4000;
		constexpr static uint32_t Overlay = 0x8000;
	};

	class G2DAPI RenderSystem : public GObject
	{
	public:
		// Begin rendering task, do some prepare jobs 
		// like clear screen and so on, all render requests
		// need to be sent after this function be called.
		virtual void BeginRender() = 0;

		// Finished rendering task, flush all requests rendering pipe.
		// It must be called after BeginRender(), all render requests
		// need to be sent before this function be called.
		virtual void EndRender() = 0;

		// Register a rendering request.
		// It provide ability to those custom components, to tell 
		// rendersytem drawing object when OnRender event occured.
		virtual void RenderMesh(uint32_t layer, Mesh*, Material*, const gml::mat32&) = 0;

		// Retrieve size of rendering window.
		virtual uint32_t GetWindowWidth() const = 0;
		virtual uint32_t GetWindowHeight() const = 0;

		// Convert screen-space coordinate to camera-space coordinate.
		virtual gml::vec2 ScreenToView(const gml::coord& screen) const = 0;

		// Convert camera-space coordinate to screen-space coordinate.
		virtual gml::coord ViewToScreen(const gml::vec2 & view) const = 0;
	};
}
