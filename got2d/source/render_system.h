#pragma once
#include "../include/g2drender.h"
#include <map>
#include <vector>
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
	bool MakeEnoughVertexArray(unsigned int numVertices);
	bool MakeEnoughIndexArray(unsigned int numIndices);
	void UploadVertices(unsigned int offset, g2d::GeometryVertex*, unsigned int count);
	void UploadIndices(unsigned int offset, unsigned int*, unsigned int count);
	void Destroy();

	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	unsigned int m_numVertices = 0;
	unsigned int m_numIndices = 0;
};

class Mesh : public g2d::Mesh
{
public:
	Mesh(unsigned int vertexCount, unsigned int indexCount);
	bool Merge(g2d::Mesh* other, const gml::mat32& transform);
	void Clear();

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

class Texture : public g2d::Texture
{
public:
	static Texture* Default() { static Texture t(""); return &t; }
	Texture(const char* resPath);
	inline const std::string& GetResourceName() { return m_resPath; }

public:
	virtual void Release() override;
private:
	std::string m_resPath;
};

class Texture2D
{
public:
	bool Create(unsigned int width, unsigned int height);
	void UploadImage(unsigned char* data, bool hasAlpha);
	void Destroy();

	ID3D11Texture2D* m_texture = nullptr;
	ID3D11ShaderResourceView* m_shaderView = nullptr;
	unsigned int m_width = 0;
	unsigned int m_height = 0;
};

class TexturePool
{
public:
	bool CreateDefaultTexture();
	void Destroy();
	Texture2D* GetTexture(const std::string& resource);
	inline Texture2D* GetDefaultTexture() { return &m_defaultTexture; }

private:
	bool LoadTextureFromFile(std::string resourcePath);

	std::map<std::string, Texture2D*> m_textures;
	Texture2D m_defaultTexture;
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

class Pass : public g2d::Pass
{
public:
	inline Pass(const char* name) : m_effectName(name) {}
	Pass(const Pass& other);
	~Pass();
	Pass* Clone();

	inline virtual const char* GetEffectName() const override { return m_effectName.c_str(); }
	virtual bool IsSame(g2d::Pass* other) const override;
	virtual void SetTexture(unsigned int index, g2d::Texture*, bool autoRelease) override;
	virtual void SetVSConstant(unsigned int index, float* data, unsigned int size, unsigned int count) override;
	virtual void SetPSConstant(unsigned int index, float* data, unsigned int size, unsigned int count) override;
	virtual g2d::Texture* GetTexture(unsigned int index) const override { return m_textures[index].texture; }
	virtual const float* GetVSConstant() const override { return reinterpret_cast<const float*>(&(m_vsConstants[0])); }
	virtual const float* GetPSConstant() const override { return reinterpret_cast<const float*>(&(m_psConstants[0])); }
	inline virtual void Release() override { delete this; }

	inline g2d::Texture* GetTextures(unsigned int index) { return m_textures[index].texture; }
	inline unsigned int GetTextureCount() { return static_cast<unsigned int>(m_textures.size()); }

private:
	std::string m_effectName;
	struct MaterialTexture
	{
		bool autoRelease;
		g2d::Texture* texture;
	};
	std::vector<MaterialTexture> m_textures;
	std::vector<gml::vec4> m_vsConstants;
	std::vector<gml::vec4> m_psConstants;
};

class Material : public g2d::Material
{
public:
	Material(unsigned int passCount);
	Material(const Material& other);
	~Material();
	void SetPass(unsigned int index, Pass* p);

public:
	virtual g2d::Pass* GetPass(unsigned int index) const override;
	virtual unsigned int GetPassCount() const override;
	virtual bool IsSame(g2d::Material* other) const override;
	virtual g2d::Material* Clone() const override;
	virtual void Release()  override;

private:
	std::vector<::Pass*> m_passes;

};

class RenderSystem : public g2d::RenderSystem
{
public:
	static RenderSystem* Instance;
	RenderSystem();

	bool Create(void* nativeWindow);

	void Destroy();

	void Clear();
	void Render();
	void Present();

	const gml::mat44& GetProjectionMatrix();

	Texture* CreateTextureFromFile(const char* resPath);

	inline ID3D11Device* GetDevice() { return m_d3dDevice; }
	inline ID3D11DeviceContext* GetContext() { return m_d3dContext; }

public:
	virtual bool OnResize(long width, long height) override;
	virtual void BeginRender() override;
	virtual void EndRender() override;
	virtual void RenderMesh(g2d::Mesh*, g2d::Material*, const gml::mat32&) override;
public:
	virtual g2d::Mesh* CreateMesh(unsigned int vertexCount, unsigned int indexCount) override;
	virtual g2d::Material* CreateDefaultMaterial() override;
	virtual g2d::Material* CreateSimpleTextureMaterial() override;
	virtual g2d::Material* CreateSimpleColorMaterial() override;


private:
	void FlushBatch();

	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11Device* m_d3dDevice = nullptr;
	ID3D11DeviceContext* m_d3dContext = nullptr;
	ID3D11Texture2D* m_colorTexture = nullptr;
	ID3D11RenderTargetView* m_rtView = nullptr;
	ID3D11RenderTargetView* m_bbView = nullptr;
	D3D11_VIEWPORT m_viewport;

	gml::color4 m_bkColor = gml::color4::blue();

	Mesh m_mesh;
	g2d::Material* m_lastMaterial = nullptr;
	ID3D11Buffer* m_bufferMatrix = nullptr;
	Geometry m_geometry;
	TexturePool m_texPool;

	ShaderLib* shaderlib = nullptr;

	gml::mat44 m_matProj;
	bool m_matrixProjDirty = true;
	long m_windowWidth = 0;
	long m_windowHeight = 0;
};


inline RenderSystem* GetRenderSystem() { return RenderSystem::Instance; }
