#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <vector>
#include "../RHI.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

class Context;

class Buffer : public rhi::Buffer
{
public:
	virtual void Release() override { delete this; }

	virtual rhi::BufferBinding GetBinding() const override { return m_bufferBinding; }

	virtual rhi::ResourceUsage GetUsage() const override { return m_bufferUsage; }

	virtual uint32_t GetLength() const override { return m_bufferLength; }

	ID3D11Buffer* GetRaw() { return &m_buffer; }

public:
	Buffer(ID3D11Buffer& buffer, rhi::BufferBinding binding, rhi::ResourceUsage usage, uint32_t length);

	~Buffer();

private:
	ID3D11Buffer& m_buffer;
	rhi::BufferBinding m_bufferBinding;
	rhi::ResourceUsage m_bufferUsage;
	const uint32_t m_bufferLength;
};

class Texture2D : public rhi::Texture2D
{
public:
	virtual void Release() override { delete this; }

	virtual uint32_t GetWidth() const override { return m_textureWidth; }

	virtual uint32_t GetHeight() const override { return m_textureHeight; }

	//temporary
	virtual ID3D11Texture2D* GetRaw() override { return &m_texture; }

public:
	Texture2D(ID3D11Texture2D& texture, uint32_t width, uint32_t height);

	~Texture2D();

private:
	ID3D11Texture2D& m_texture;
	const uint32_t m_textureWidth;
	const uint32_t m_textureHeight;

};

class SwapChain : public rhi::SwapChain
{
public:
	virtual void Release() override { delete this; }

	virtual rhi::Texture2D* GetBackBuffer() override;

	virtual uint32_t GetWidth() const override { return m_windowWidth; }

	virtual uint32_t GetHeight() const override { return m_windowHeight; }

	virtual bool ResizeBackBuffer(uint32_t width, uint32_t height) override;

	virtual void Present() override;

	virtual IDXGISwapChain* GetRaw() { return &m_swapChain; }

public:
	SwapChain(IDXGISwapChain& swapChain);

	~SwapChain();

	void UpdateWindowSize();

private:
	IDXGISwapChain& m_swapChain;
	uint32_t m_windowWidth;
	uint32_t m_windowHeight;
};

class Device : public rhi::Device
{
public:
	virtual void Release() override { delete this; }

	virtual rhi::SwapChain* CreateSwapChain(void* nativeWindow, uint32_t windowWidth, uint32_t windowHeight) override;

	virtual rhi::Buffer* CreateBuffer(rhi::BufferBinding binding, rhi::ResourceUsage usage, uint32_t bufferLength) override;

	virtual rhi::Texture2D* CreateTexture2D(rhi::TextureFormat format, rhi::ResourceUsage usage, uint32_t binding, uint32_t width, uint32_t height) override;

	virtual ID3D11Device* GetRaw() { return &m_d3dDevice; }

public:
	Device(ID3D11Device& d3dDevice);

	~Device();

private:
	ID3D11Device& m_d3dDevice;
};

class Context : public rhi::Context
{
public:
	virtual void Release() override { delete this; }

	virtual void SetVertexBuffers(uint32_t startSlot, rhi::VertexBufferInfo* buffers, uint32_t bufferCount) override;

	virtual void SetIndexBuffer(rhi::Buffer* buffer, uint32_t offset, rhi::IndexFormat format) override;

	virtual void SetVertexShaderConstantBuffers(uint32_t startSlot, rhi::Buffer** buffers, uint32_t bufferCount) override;

	virtual void SetPixelShaderConstantBuffers(uint32_t startSlot, rhi::Buffer** buffers, uint32_t bufferCount) override;

	virtual void DrawIndexed(rhi::Primitive primitive, uint32_t startIndex, uint32_t indexOffset, uint32_t baseVertex) override;

	virtual rhi::MappedResource Map(rhi::Buffer* buffer) override;

	virtual void Unmap(rhi::Buffer* buffer) override;

	virtual ID3D11DeviceContext* GetRaw() { return &m_d3dContext; }

public:
	Context(ID3D11DeviceContext& d3dContext);

	~Context();

private:
	ID3D11DeviceContext& m_d3dContext;
	std::vector<ID3D11Buffer*> m_vertexbuffers;
	std::vector<ID3D11Buffer*> m_vsConstantBuffers;
	std::vector<ID3D11Buffer*> m_psConstantBuffers;
	std::vector<UINT> m_vertexBufferStrides;
	std::vector<UINT> m_vertexBufferOffsets;

};