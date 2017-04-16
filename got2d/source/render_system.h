#pragma once
#include <map>
#include <vector>
#include <gml/gmlcolor.h>
#include "../include/g2drender.h"
#include "inner_utility.h"
#include "scope_utility.h"
#include "../RHI/RHI.h"

class Geometry
{
public:
	bool Create(uint32_t vertexCount, uint32_t indexCount);

	bool MakeEnoughVertexArray(uint32_t numVertices);

	bool MakeEnoughIndexArray(uint32_t numIndices);

	void UploadVertices(uint32_t offset, g2d::GeometryVertex*, uint32_t count);

	void UploadIndices(uint32_t offset, uint32_t* indices, uint32_t count);

	void Destroy();

	autor<rhi::Buffer> m_vertexBuffer = nullptr;
	autor<rhi::Buffer> m_indexBuffer = nullptr;
	uint32_t m_numVertices = 0;
	uint32_t m_numIndices = 0;
};

class Mesh : public g2d::Mesh
{
	RTTI_IMPL;
public:
	Mesh(uint32_t vertexCount, uint32_t indexCount);

	void Clear();

	bool Merge(const g2d::Mesh& other, const gml::mat32& transform);

public:
	virtual const g2d::GeometryVertex* GetRawVertices() const override;

	virtual g2d::GeometryVertex* GetRawVertices() override;

	virtual const uint32_t* GetRawIndices() const override;

	virtual uint32_t* GetRawIndices() override;

	virtual uint32_t GetVertexCount() const override;

	virtual uint32_t GetIndexCount() const override;

	virtual void ResizeVertexArray(uint32_t vertexCount) override;

	virtual void ResizeIndexArray(uint32_t indexCount) override;

	virtual bool Merge(g2d::Mesh* other, const gml::mat32& transform) override;

	virtual void Release() override;

private:
	std::vector<g2d::GeometryVertex> m_vertices;
	std::vector<uint32_t> m_indices;
};

class Texture : public g2d::Texture
{
	RTTI_IMPL;
public:
	static Texture& Default() { static Texture t(""); return t; }

	Texture(std::string resPath);

	const std::string& GetResourceName() const { return m_resPath; }

public: // g2d::Texture
	virtual void Release() override;

	virtual const char* Identifier() const override { return m_resPath.c_str(); }

	virtual bool IsSame(g2d::Texture* other) const override;

	virtual void AddRef() override;

private:
	int m_refCount = 1;
	std::string m_resPath;
};

class Texture2D
{
public:
	bool Create(uint32_t width, uint32_t height);

	void UploadImage(uint8_t* data, bool hasAlpha);

	void Destroy();

	autor<ID3D11Texture2D> m_texture = nullptr;
	autor<ID3D11ShaderResourceView> m_shaderView = nullptr;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
};

class TexturePool
{
public:
	bool CreateDefaultTexture();

	void Destroy();

	Texture2D* GetTexture(const std::string& resource);

	Texture2D& GetDefaultTexture() { return m_defaultTexture; }

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

	virtual uint32_t GetConstBufferLength() = 0;
};

class PSData
{
public:
	virtual ~PSData() {}

	virtual const char* GetName() = 0;

	virtual const char* GetCode() = 0;

	virtual uint32_t GetConstBufferLength() = 0;
};

class Shader
{
	RTTI_INNER_IMPL;
public:
	bool Create(const std::string& vsCode, uint32_t vcbLength, const std::string& psCode, uint32_t pcbLength);

	void Destroy();

	ID3D11VertexShader* GetVertexShader() { return m_vertexShader; }

	ID3D11PixelShader* GetPixelShader() { return m_pixelShader; }

	ID3D11InputLayout* GetInputLayout() { return m_shaderLayout; }

	rhi::Buffer* GetVertexConstBuffer() { return m_vertexConstBuffer; }

	rhi::Buffer* GetPixelConstBuffer() { return m_pixelConstBuffer; }

	uint32_t GetVertexConstBufferLength() { return m_vertexConstBufferLength; }

	uint32_t GetPixelConstBufferLength() { return m_pixelConstBufferLength; }

private:
	autor<ID3D11InputLayout> m_shaderLayout = nullptr;
	autor<ID3D11VertexShader>  m_vertexShader = nullptr;
	autor<ID3D11PixelShader> m_pixelShader = nullptr;
	autor<rhi::Buffer> m_vertexConstBuffer = nullptr;
	autor<rhi::Buffer> m_pixelConstBuffer = nullptr;
	uint32_t m_vertexConstBufferLength = 0;
	uint32_t m_pixelConstBufferLength = 0;
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
	RTTI_IMPL;
public:
	Pass(std::string vsName, std::string psName)
		: m_vsName(std::move(vsName))
		, m_psName(std::move(psName))
		, m_blendMode(g2d::BlendMode::None) {}

	Pass(const Pass& other);

	~Pass();

	Pass* Clone();

	void Release() { delete this; }

public:
	virtual const char* GetVertexShaderName() const override { return m_vsName.c_str(); }

	virtual const char* GetPixelShaderName() const override { return m_psName.c_str(); }

	virtual bool IsSame(g2d::Pass* other) const override;

	virtual void SetTexture(uint32_t index, g2d::Texture*, bool autoRelease) override;

	virtual void SetVSConstant(uint32_t index, float* data, uint32_t size, uint32_t count) override;

	virtual void SetPSConstant(uint32_t index, float* data, uint32_t size, uint32_t count) override;

	virtual void SetBlendMode(g2d::BlendMode blendMode) override { m_blendMode = blendMode; }

	virtual g2d::Texture* GetTextureByIndex(uint32_t index) const override { return m_textures[index]; }

	virtual uint32_t GetTextureCount() const override { return static_cast<uint32_t>(m_textures.size()); }

	virtual const float* GetVSConstant() const override { return reinterpret_cast<const float*>(&(m_vsConstants[0])); }

	virtual uint32_t GetVSConstantLength() const override { return static_cast<uint32_t>(m_vsConstants.size()) * 4 * sizeof(float); }

	virtual const float* GetPSConstant() const override { return reinterpret_cast<const float*>(&(m_psConstants[0])); }

	virtual uint32_t GetPSConstantLength() const override { return static_cast<uint32_t>(m_psConstants.size()) * 4 * sizeof(float); }

	virtual g2d::BlendMode GetBlendMode() const override { return m_blendMode; }

private:
	std::string m_vsName = "";
	std::string m_psName = "";
	std::vector<g2d::Texture*> m_textures;
	std::vector<gml::vec4> m_vsConstants;
	std::vector<gml::vec4> m_psConstants;
	g2d::BlendMode m_blendMode = g2d::BlendMode::None;
};

class Material : public g2d::Material
{
	RTTI_IMPL;
public:
	Material(uint32_t passCount);

	Material(const Material& other);

	~Material();

	void SetPass(uint32_t index, Pass* p);

public:
	virtual g2d::Pass* GetPassByIndex(uint32_t index) const override;

	virtual uint32_t GetPassCount() const override;

	virtual bool IsSame(g2d::Material* other) const override;

	virtual g2d::Material* Clone() const override;

	virtual void Release()  override;

private:
	std::vector<::Pass*> m_passes;

};

class RenderSystem : public g2d::RenderSystem
{
	RTTI_IMPL;
public:
	static RenderSystem* Instance;

	bool Create(void* nativeWindow);

	void Destroy();

	void Clear();

	void FlushRequests();

	void Present();

	void SetBlendMode(g2d::BlendMode blendMode);

	void SetViewMatrix(const gml::mat32& viewMatrix);

	const gml::mat44& GetProjectionMatrix();

	Texture* CreateTextureFromFile(const char* resPath);

	rhi::Device* GetDevice() { return m_device; }

	rhi::Context* GetContext() { return m_context; }

	bool OnResize(uint32_t width, uint32_t height);

public:
	virtual void BeginRender() override;

	virtual void EndRender() override;

	virtual void RenderMesh(uint32_t layer, g2d::Mesh*, g2d::Material*, const gml::mat32&) override;

	virtual uint32_t GetWindowWidth() const override { return m_windowWidth; }

	virtual uint32_t GetWindowHeight() const override { return m_windowHeight; }

	virtual gml::vec2 ScreenToView(const gml::coord& screen) const override;

	virtual gml::coord ViewToScreen(const gml::vec2 & view) const override;

private:
	bool CreateBlendModes();

	void FlushBatch(Mesh& mesh, g2d::Material&);

	void UpdateConstBuffer(rhi::Buffer* cbuffer, const void* data, uint32_t length);

	void UpdateSceneConstBuffer();

	autor<rhi::Device> m_device = nullptr;
	autor<rhi::Context> m_context = nullptr;
	autor<rhi::SwapChain> m_swapChain = nullptr;

	autor<ID3D11Texture2D> m_colorTexture = nullptr;
	autor<ID3D11RenderTargetView> m_rtView = nullptr;
	autor<ID3D11RenderTargetView> m_bbView = nullptr;
	autor<rhi::Buffer> m_sceneConstBuffer = nullptr;
	D3D11_VIEWPORT m_viewport;
	std::map<g2d::BlendMode, ID3D11BlendState*> m_blendModes;

	gml::color4 m_bkColor = gml::color4::blue();

	//render request
	struct RenderRequest {
		RenderRequest(g2d::Mesh& inMesh, g2d::Material& inMaterial, const gml::mat32& inWorldMatrix)
			: mesh(inMesh), material(inMaterial), worldMatrix(inWorldMatrix)
		{	}
		g2d::Mesh& mesh;
		g2d::Material& material;
		gml::mat32 worldMatrix = gml::mat32::identity();
	};

	typedef std::vector<RenderRequest> ReqList;
	std::map<uint32_t, ReqList*> m_renderRequests;

	Geometry m_geometry;
	TexturePool m_texPool;
	autod<ShaderLib> m_shaderlib = nullptr;
	gml::mat32 m_matView = gml::mat32::identity();
	gml::mat44 m_matProj = gml::mat44::identity();
	bool m_matrixConstBufferDirty = true;
	bool m_matrixProjDirty = true;
	uint32_t m_windowWidth = 0;
	uint32_t m_windowHeight = 0;
};


inline RenderSystem* GetRenderSystem() { return RenderSystem::Instance; }
