#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <cinttypes>

namespace rhi
{
	enum class BufferBinding : int
	{
		Vertex = 0,
		Index = 1,
		Constant = 2,
	};

	enum class ResourceUsage : int
	{
		Default = 0,
		Dynamic = 1,
	};

	enum class IndexFormat : int
	{
		Int16 = 0,
		Int32 = 1,
	};

	enum class Primitive : int
	{
		TriangleList = 0,
		TriangleStrip = 1,
	};

	enum class TextureFormat : int
	{
		RGBA = 0, BGRA = 1,
		DXT1 = 2, DXT3 = 3, DXT5 = 4,
		Float32 = 5,
	};

	class TextureBinding
	{
	public:
		constexpr static int ShaderResource = 1 << 0;
		constexpr static int RenderTarget = 1 << 1;
		constexpr static int DepthStencil = 1 << 2;
		constexpr static int StreamOutput = 1 << 3;
		constexpr static int Unordered = 1 << 4;
		constexpr static int Count = 5;
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
		virtual rhi::BufferBinding GetBinding() const = 0;

		virtual rhi::ResourceUsage GetUsage() const = 0;

		virtual uint32_t GetLength() const = 0;
	};

	class Texture2D : public RHIObject
	{
	public:
		virtual uint32_t GetWidth() const = 0;

		virtual uint32_t GetHeight() const = 0;

		//temporary
		virtual ID3D11Texture2D* GetRaw() = 0;
	};

	class SwapChain : public RHIObject
	{
	public:
		virtual Texture2D* GetBackBuffer() = 0;

		virtual uint32_t GetWidth() const = 0;

		virtual uint32_t GetHeight() const = 0;

		virtual bool ResizeBackBuffer(uint32_t width, uint32_t height) = 0;

		virtual void Present() = 0;

		// temporary
		virtual IDXGISwapChain* GetRaw() = 0;
	};

	class Device : public RHIObject
	{
	public:
		virtual SwapChain* CreateSwapChain(void* nativeWindow, uint32_t windowWidth, uint32_t windowHeight) = 0;

		virtual Buffer* CreateBuffer(BufferBinding binding, ResourceUsage usage, uint32_t bufferLength) = 0;

		virtual Texture2D* CreateTexture2D(TextureFormat format, ResourceUsage usage, uint32_t binding, uint32_t width, uint32_t height) = 0;

		// temporary
		virtual ID3D11Device* GetRaw() = 0;
	};

	struct MappedResource
	{
		bool success = false;
		void* data = nullptr;
		uint32_t linePitch = 0;
	};

	struct VertexBufferInfo
	{
		Buffer* buffer = nullptr;
		uint32_t stride = 0;
		uint32_t offset = 0;
	};

	class Context :public RHIObject
	{
	public:
		virtual void SetVertexBuffers(uint32_t startSlot, VertexBufferInfo* buffers, uint32_t bufferCount) = 0;

		virtual void SetIndexBuffer(Buffer* buffer, uint32_t offset, IndexFormat format) = 0;

		virtual void SetVertexShaderConstantBuffers(uint32_t startSlot, Buffer** buffers, uint32_t bufferCount) = 0;

		virtual void SetPixelShaderConstantBuffers(uint32_t startSlot, Buffer** buffers, uint32_t bufferCount) = 0;

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