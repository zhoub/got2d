#pragma once
#include "../include/g2drender.h"
#include <map>
#include <windows.h>
#include <d3d11.h>
#include <gmlcolor.h>
#include <vector>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")


class Geometry
{
public:
	bool Create(unsigned int vertexCount, unsigned int indexCount);
	void UploadVertices(g2d::GeometryVertex*);
	void UploadIndices(unsigned int*);
	void Destroy();

	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	unsigned int m_vertexCount = 0;
	unsigned int m_indexCount = 0;
};

class ShaderSource
{
public:
	virtual const char* GetShaderName() = 0;
	virtual const char* GetVertexShaderCode() = 0;
	virtual const char* GetPixelShaderCode() = 0;
};

class Shader
{
public:
	bool Create(const char* vsCode, const char* psCode);
	void Destroy();

	ID3D11VertexShader* GetVertexShader();
	ID3D11PixelShader* GetPixelShader();
	ID3D11InputLayout* GetInputLayout();

private:
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_shaderLayout = nullptr;
};

class ShaderLib
{
public:
	ShaderLib();
	Shader* GetShaderByName(const char* name);

private:
	bool BuildShader(const std::string& name);

	std::map<std::string, ShaderSource*> m_sources;
	std::map<std::string, Shader*> m_shaders;
};

class Mesh : public g2d::Mesh
{
public:
	Mesh(unsigned int vertexCount, unsigned int indexCount);

	virtual g2d::GeometryVertex* GetRawVertices() override;
	virtual unsigned int* GetRawIndices() override;
	virtual unsigned int GetVertexCount() override;
	virtual unsigned int GetIndexCount() override;
	virtual void ResizeVertexArray(unsigned int vertexCount) override;
	virtual void ResizeIndexArray(unsigned int indexCount) override;
	virtual void Release() override;
private:
	std::vector<g2d::GeometryVertex> m_vertices;
	std::vector<unsigned int> m_indices;
};
class RenderSystem : public g2d::RenderSystem
{
public:
	static RenderSystem* Instance;
	RenderSystem();

	bool Create(void* nativeWindow);
	bool OnResize(int width, int height);
	void Destroy();

	void Clear();
	void Render();
	void Present();

	inline ID3D11Device* GetDevice() { return m_d3dDevice; }
	inline ID3D11DeviceContext* GetContext() { return m_d3dContext; }

public:
	virtual void BeginRender() override;
	virtual void EndRender() override;
	virtual g2d::Mesh* CreateMesh(unsigned int vertexCount, unsigned int indexCount) override;
	virtual void RenderMesh(g2d::Mesh*) override;

private:
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11Device* m_d3dDevice = nullptr;
	ID3D11DeviceContext* m_d3dContext = nullptr;
	ID3D11Texture2D* m_colorTexture = nullptr;
	ID3D11RenderTargetView* m_rtView = nullptr;
	ID3D11RenderTargetView* m_bbView = nullptr;
	D3D11_VIEWPORT m_viewport;

	gml::color4 m_bkColor;


	Geometry m_geometry;

	ShaderLib* shaderlib = nullptr;
};


inline RenderSystem* GetRenderSystem() { return RenderSystem::Instance; }
