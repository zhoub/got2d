#pragma once
#include <Windows.h>
#include <d3d11.h>
#include "../RHI.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

class Context;

class Buffer : public rhi::Buffer
{
public:
	virtual void Release() override { delete this; }

	virtual rhi::BufferBinding GetBinding() const override { return m_bufferBinding; }

	virtual rhi::BufferUsage GetUsage() const override { return m_bufferUsage; }

	virtual uint32_t GetLength() const override { return m_bufferLength; }

	virtual ID3D11Buffer* GetRaw() { return &m_buffer; }

public:
	Buffer(ID3D11Buffer& buffer, rhi::BufferBinding binding, rhi::BufferUsage usage, uint32_t length);

	~Buffer();

private:
	ID3D11Buffer& m_buffer;
	rhi::BufferBinding m_bufferBinding;
	rhi::BufferUsage m_bufferUsage;
	uint32_t m_bufferLength;
};

class SwapChain : public rhi::SwapChain
{
public:
	virtual void Release() override { delete this; }

	virtual gml::rect GetRect() override;

	virtual void Present() override;

	virtual IDXGISwapChain* GetRaw() { return &m_swapChain; }

public:
	SwapChain(IDXGISwapChain& swapChain);

	~SwapChain();

private:
	IDXGISwapChain& m_swapChain;
};

class Device : public rhi::Device
{
public:
	virtual void Release() override { delete this; }

	virtual rhi::SwapChain* CreateSwapChain(void* nativeWindow, uint32_t windowWidth, uint32_t windowHeight) override;

	virtual rhi::Buffer* CreateBuffer(rhi::BufferBinding binding, rhi::BufferUsage usage, uint32_t bufferLength) override;

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

	virtual rhi::MappedResource Map(rhi::Buffer* buffer) override;

	virtual void Unmap(rhi::Buffer* buffer) override;

	ID3D11DeviceContext& GetContext() { return m_d3dContext; }

	virtual ID3D11DeviceContext* GetRaw() { return &m_d3dContext; }

public:
	Context(ID3D11DeviceContext& d3dContext);

	~Context();

private:
	ID3D11DeviceContext& m_d3dContext;
};