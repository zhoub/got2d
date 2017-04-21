#pragma once
#include <cinttypes>
#include <gml/gmlcolor.h>
#include <gml/gmlvector.h>
#include "../source/scope_utility.h"

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
		Unknown = 0,
		RGBA = 1, BGRA = 2,
		DXT1 = 3, DXT3 = 4, DXT5 = 5,
		D24S8 = 6, Float32 = 7,
	};

	enum class InputFormat : int
	{
		Float2, Float3, Float4,
	};

	enum class BlendFactor : int
	{
		Zero = 0,
		One = 1,
		SourceColor = 2,
		SourceAlpha = 3,
		DestinationColor = 4,
		DestinationAlpha = 5,
		InverseSourceColor = 6,
		InverseSourceAlpha = 7,
		InverseDestinationColor = 8,
		InverseDestinationAlpha = 9,
	};

	enum class BlendOperator : int
	{
		Add = 0, Sub = 1,
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

	enum class SamplerFilter : int
	{
		MinMagMipPoint,
		MinMagMipLinear,
	};

	enum class TextureAddress : int
	{
		Repeat = 0,
		Clamp = 1,
		Mirror = 2,
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

	template<typename T, T TMin, T TMax> struct CountRange
	{
		const T Count;
		constexpr static T Min = TMin;
		constexpr static T Max = TMax;
		CountRange(T c) : Count(c) { ENSURE(Count >= TMin && Count <= TMax); }
		operator T() const { return Count; }
	};

	template<uint32_t TEnd> struct Index
	{
		const uint32_t Value;
		constexpr static uint32_t End = TEnd;
		constexpr static uint32_t Max = TEnd - 1;
		Index(uint32_t i) : Value(i) { ENSURE(Value < TEnd); }
		operator uint32_t() const { return Value; }
	};

	constexpr uint32_t MinRenderTarget = 1;
	constexpr uint32_t MaxRenderTarget = 4;
	typedef CountRange<uint32_t, MinRenderTarget, MaxRenderTarget> RTCountRange;
	typedef Index<MaxRenderTarget> RTIndex;

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

		virtual TextureFormat GetFormat() const = 0;

		virtual bool IsRenderTarget() const = 0;

		virtual bool IsShaderResource() const = 0;

		virtual bool IsDepthStencil() const = 0;
	};

	class RenderTarget : public RHIObject
	{
	public:
		virtual Texture2D* GetColorBufferByIndex(RTIndex index) = 0;

		virtual uint32_t GetColorBufferCount() = 0;

		// return nullptr if depth/stencil is not enabled.
		virtual Texture2D* GetDepthStencilBuffer() = 0;

		virtual bool IsDepthStencilUsed() const = 0;

		virtual uint32_t GetWidth() const = 0;

		virtual uint32_t GetHeight() const = 0;
	};

	class ShaderProgram : public RHIObject { };

	class BlendState : public RHIObject
	{
	public:
		virtual bool IsEnabled() const = 0;

		virtual BlendFactor GetSourceFactor() const = 0;

		virtual BlendFactor GetDestinationFactor() const = 0;

		virtual BlendOperator GetOperator() const = 0;
	};

	class TextureSampler : public RHIObject { };

	class SwapChain : public RHIObject
	{
	public:
		virtual RenderTarget* GetBackBuffer() = 0;

		virtual uint32_t GetWidth() const = 0;

		virtual uint32_t GetHeight() const = 0;

		virtual bool OnResize(uint32_t width, uint32_t height) = 0;

		virtual void SetFullscreen(bool fullscreen) = 0;

		virtual bool IsFullscreen() const = 0;

		virtual void Present() = 0;
	};

	struct InputLayout
	{
		const char* SemanticName = "";
		uint32_t SemanticIndex = 0;
		uint32_t InputSlot = 0;
		uint32_t AlignOffset = 0xFFFFFFFF;
		InputFormat Format = InputFormat::Float4;
		bool IsInstanced = false;
		uint32_t InstanceRepeatRate = 1;
	};

	class Device : public RHIObject
	{
	public:
		// if width / height is specified as zero, default size of native window will be used.
		virtual SwapChain* CreateSwapChain(void* nativeWindow, bool useDepthStencil, uint32_t windowWidth, uint32_t windowHeight) = 0;

		virtual Buffer* CreateBuffer(BufferBinding binding, ResourceUsage usage, uint32_t bufferLength) = 0;

		virtual Texture2D* CreateTexture2D(TextureFormat format, ResourceUsage usage, uint32_t binding, uint32_t width, uint32_t height) = 0;

		virtual ShaderProgram* CreateShaderProgram(
			const char* vsSource, const char* vsEntry,
			const char* psSource, const char* psEntry,
			InputLayout* layouts, uint32_t layoutCount) = 0;

		virtual BlendState* CreateBlendState(bool enabled, BlendFactor source, BlendFactor dest, BlendOperator op) = 0;

		virtual TextureSampler* CreateTextureSampler(SamplerFilter filter, TextureAddress addressU, TextureAddress addressV) = 0;

		virtual RenderTarget* CreateRenderTarget(uint32_t width, uint32_t height, TextureFormat* rtFormats, RTCountRange rtCount, bool useDpethStencil) = 0;
	};

	struct Viewport
	{
		gml::vec2 LTPosition;
		gml::vec2 Size;
		gml::vec2 MinMaxZ;
	};

	struct VertexBufferInfo
	{
		Buffer* buffer = nullptr;
		uint32_t stride = 0;
		uint32_t offset = 0;
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
		virtual void ClearRenderTarget(RenderTarget* renderTarget, gml::color4 clearColor) = 0;

		virtual void SetViewport(const Viewport& viewport) = 0;

		virtual void SetRenderTarget(RenderTarget* renderTarget) = 0;

		virtual void SetVertexBuffers(uint32_t startSlot, VertexBufferInfo* buffers, uint32_t bufferCount) = 0;

		virtual void SetIndexBuffer(Buffer* buffer, uint32_t offset, IndexFormat format) = 0;

		virtual void SetShaderProgram(ShaderProgram* program) = 0;

		virtual void SetVertexShaderConstantBuffers(uint32_t startSlot, Buffer** buffers, uint32_t bufferCount) = 0;

		virtual void SetPixelShaderConstantBuffers(uint32_t startSlot, Buffer** buffers, uint32_t bufferCount) = 0;

		virtual void SetTextures(uint32_t startSlot, Texture2D** textures, uint32_t resCount) = 0;

		virtual void SetBlendState(BlendState* state) = 0;

		virtual void SetTextureSampler(uint32_t startSlot, TextureSampler** samplers, uint32_t count) = 0;

		virtual void DrawIndexed(Primitive primitive, uint32_t indexCount, uint32_t startIndex, uint32_t baseVertex) = 0;

		virtual MappedResource Map(Buffer* buffer) = 0;

		virtual MappedResource Map(Texture2D* buffer) = 0;

		virtual void Unmap(Buffer* buffer) = 0;

		virtual void Unmap(Texture2D* buffer) = 0;

		virtual void GenerateMipmaps(Texture2D* texture) = 0;
	};

	struct RHICreationResult
	{
		bool Success = false;
		Device* DevicePtr = nullptr;
		Context* ContextPtr = nullptr;
	};

	RHICreationResult CreateRHI();
}