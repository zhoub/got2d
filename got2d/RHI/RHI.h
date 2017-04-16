#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <cinttypes>
#include <gml/gmlrect.h>

namespace rhi
{
	enum class BufferBinding
	{
		Vertex = 0,
		Index = 1,
		Constant = 2,
	};

	enum class BufferUsage
	{
		Default = 0,
		Dynamic = 1,
	};

	enum class IndexFormat
	{
		Int16 = 0,
		Int32 = 1,
	};

	enum class Primitive
	{
		TriangleList = 0,
		TriangleStrip = 1,
	};

	class RHIObject
	{
	public:
		RHIObject() = default;
		virtual ~RHIObject() { }
		virtual void Release() = 0;
	private:
		RHIObject(const RHIObject&) = delete;
		RHIObject(const RHIObject&&) = delete;
		RHIObject& operator=(const RHIObject&) = delete;
	};

	class Buffer : public RHIObject
	{
	public:
		virtual ID3D11Buffer* GetRaw() = 0;

		virtual rhi::BufferBinding GetBinding() const = 0;

		virtual rhi::BufferUsage GetUsage() const = 0;

		virtual uint32_t GetLength() const = 0;
	};

	class SwapChain : public RHIObject
	{
	public:
		virtual gml::rect GetRect() = 0;

		virtual bool ResizeBackBuffer(uint32_t width, uint32_t height) = 0;

		virtual void Present() = 0;

		// temporary
		virtual IDXGISwapChain* GetRaw() = 0;
	};

	class Device : public RHIObject
	{
	public:
		virtual SwapChain* CreateSwapChain(void* nativeWindow, uint32_t windowWidth, uint32_t windowHeight) = 0;

		virtual Buffer* CreateBuffer(BufferBinding binding, BufferUsage usage, uint32_t bufferLength) = 0;

		// temporary
		virtual ID3D11Device* GetRaw() = 0;
	};
	
	struct MappedResource
	{
		bool success = false;
		void* data = nullptr;
		uint32_t linePitch = 0;
	};

	class Context :public RHIObject
	{
	public:
		//virtual void SetVertexBuffers() = 0;

		//virtual void SetIndexBuffer(Buffer* buffer, uint32_t offset, IndexFormat format) = 0;

		virtual void DrawIndexed(Primitive primitive, uint32_t indexCount, uint32_t startIndex, uint32_t baseVertex) = 0;

		virtual MappedResource Map(Buffer* buffer) = 0;

		virtual void Unmap(Buffer* buffer) = 0;

		// temporary
		virtual ID3D11DeviceContext* GetRaw() = 0;
	};

	struct RHICreationResult
	{
		bool success = false;
		Device* device = nullptr;
		Context* context = nullptr;
	};

	RHICreationResult CreateRHI();
}