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
	virtual bool IsSame(g2d::Texture* other) const override;
	virtual void AddRef() override;
	virtual void Release() override;
private:
	int m_refCount = 1;
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

class VSData
{
public:
	virtual ~VSData() {}
	virtual const char* GetName() = 0;
	virtual const char* GetCode() = 0;
	virtual unsigned int GetConstBufferLength() = 0;
};

class PSData
{
public:
	virtual ~PSData() {}
	virtual const char* GetName() = 0;
	virtual const char* GetCode() = 0;
	virtual unsigned int GetConstBufferLength() = 0;
};

class Shader
{
public:
	bool Create(const char* vsCode, unsigned int vcbLength, const char* psCode, unsigned int pcbLength);
	void Destroy();

	ID3D11VertexShader* GetVertexShader() { return m_vertexShader; }
	ID3D11PixelShader* GetPixelShader() { return m_pixelShader; }
	ID3D11InputLayout* GetInputLayout() { return m_shaderLayout; }
	ID3D11Buffer* GetVertexConstBuffer() { return m_vertexConstBuffer; }
	ID3D11Buffer* GetPixelConstBuffer() { return m_pixelConstBuffer; }
	unsigned int GetVertexConstBufferLength() { return m_vertexConstBufferLength; }
	unsigned int GetPixelConstBufferLength() { return m_pixelConstBufferLength; }

private:
	ID3D11InputLayout* m_shaderLayout = nullptr;
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11Buffer* m_vertexConstBuffer = nullptr;
	ID3D11Buffer* m_pixelConstBuffer = nullptr;
	unsigned int m_vertexConstBufferLength = 0;
	unsigned int m_pixelConstBufferLength = 0;
};

class ShaderLib
{
public:
	ShaderLib();
	~ShaderLib();
	Shader* GetShaderByName(const std::string& vsName, const std::string& psName);

private:
	bool BuildShader(const std::string& effectName, const std::string& vsName, const std::string& psName);
	std::string GetEffectName(const std::string& vsName, const std::string& psName);
	std::map<std::string, VSData*> m_vsSources;
	std::map<std::string, PSData*>  m_psSources;
	std::map<std::string, Shader*> m_shaders;
};

class Pass : public g2d::Pass
{
public:
	inline Pass(const char* vsName, const char* psName) : m_vsName(vsName), m_psName(psName), m_blendMode(g2d::BLEND_NONE) {}
	Pass(const Pass& other);
	~Pass();
	Pass* Clone();

	inline virtual const char* GetVertexShaderName() const override { return m_vsName.c_str(); }
	inline virtual const char* GetPixelShaderName() const override { return m_psName.c_str(); }
	virtual bool IsSame(g2d::Pass* other) const override;
	virtual void SetTexture(unsigned int index, g2d::Texture*, bool autoRelease) override;
	virtual void SetVSConstant(unsigned int index, float* data, unsigned int size, unsigned int count) override;
	virtual void SetPSConstant(unsigned int index, float* data, unsigned int size, unsigned int count) override;
	inline virtual void SetBlendMode(g2d::BlendMode blendMode) override { m_blendMode = blendMode; }
	virtual g2d::Texture* GetTexture(unsigned int index) const override { return m_textures[index]; }
	inline virtual unsigned int GetTextureCount() const override { return static_cast<unsigned int>(m_textures.size()); }
	inline virtual const float* GetVSConstant() const override { return reinterpret_cast<const float*>(&(m_vsConstants[0])); }
	inline virtual unsigned int GetVSConstantLength() const override { return static_cast<unsigned int>(m_vsConstants.size()) * 4 * sizeof(float); }
	inline virtual const float* GetPSConstant() const override { return reinterpret_cast<const float*>(&(m_psConstants[0])); }
	inline virtual unsigned int GetPSConstantLength() const override { return static_cast<unsigned int>(m_psConstants.size()) * 4 * sizeof(float); }
	inline virtual void Release() override { delete this; }
	virtual g2d::BlendMode GetBlendMode() const override;

private:
	std::string m_vsName;
	std::string m_psName;
	std::vector<g2d::Texture*> m_textures;
	std::vector<gml::vec4> m_vsConstants;
	std::vector<gml::vec4> m_psConstants;
	g2d::BlendMode m_blendMode;
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
	void SetBlendMode(g2d::BlendMode blendMode);
	const gml::mat44& GetProjectionMatrix();

	Texture* CreateTextureFromFile(const char* resPath);

	inline ID3D11Device* GetDevice() { return m_d3dDevice; }
	inline ID3D11DeviceContext* GetContext() { return m_d3dContext; }

public:
	virtual bool OnResize(long width, long height) override;
	virtual void BeginRender() override;
	virtual void EndRender() override;
	virtual void RenderMesh(unsigned int layer, g2d::Mesh*, g2d::Material*, const gml::mat32&) override;
public:
	virtual g2d::Mesh* CreateMesh(unsigned int vertexCount, unsigned int indexCount) override;
	virtual g2d::Material* CreateColorTextureMaterial() override;
	virtual g2d::Material* CreateSimpleTextureMaterial() override;
	virtual g2d::Material* CreateSimpleColorMaterial() override;

private:
	bool CreateBlendModes();
	void FlushBatch(Mesh& mesh, g2d::Material*);
	void UpdateConstBuffer(ID3D11Buffer* cbuffer, const void* data, unsigned int length);
	void UpdateSceneConstBuffer(gml::mat32* matrixView);

	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11Device* m_d3dDevice = nullptr;
	ID3D11DeviceContext* m_d3dContext = nullptr;
	ID3D11Texture2D* m_colorTexture = nullptr;
	ID3D11RenderTargetView* m_rtView = nullptr;
	ID3D11RenderTargetView* m_bbView = nullptr;
	D3D11_VIEWPORT m_viewport;
	std::map<g2d::BlendMode, ID3D11BlendState*> m_blendModes;

	gml::color4 m_bkColor = gml::color4::blue();

	//render request
	struct RenderRequest {
		RenderRequest(g2d::Mesh* inMesh, g2d::Material* inMaterial, const gml::mat32& inWorldMatrix)
			: mesh(inMesh)
			, material(inMaterial)
			, worldMatrix(inWorldMatrix)
		{

		}
		g2d::Mesh* mesh;
		g2d::Material* material;
		gml::mat32 worldMatrix;
	};

	typedef std::vector<RenderRequest> ReqList;
	std::map<unsigned int, ReqList*> m_renderRequests;

	Geometry m_geometry;
	TexturePool m_texPool;
	ShaderLib* shaderlib = nullptr;
	ID3D11Buffer* m_sceneConstBuffer = nullptr;
	gml::mat44 m_matProj;

	bool m_matProjConstBufferDirty = true;
	bool m_matrixProjDirty = true;
	long m_windowWidth = 0;
	long m_windowHeight = 0;
};


inline RenderSystem* GetRenderSystem() { return RenderSystem::Instance; }
