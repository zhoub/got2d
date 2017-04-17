#include <d3dcompiler.h>
#include "render_system.h"

#pragma comment(lib,"d3dcompiler.lib")

g2d::Material* g2d::Material::CreateColorTexture()
{
	auto mat = new ::Material(1);
	mat->SetPass(0, new ::Pass("default", "color.texture"));
	mat->GetPassByIndex(0)->SetTexture(0, &::Texture::Default(), false);
	return mat;
}

g2d::Material* g2d::Material::CreateSimpleTexture()
{
	auto mat = new ::Material(1);
	mat->SetPass(0, new ::Pass("default", "simple.texture"));
	mat->GetPassByIndex(0)->SetTexture(0, &::Texture::Default(), false);
	return mat;
}

g2d::Material* g2d::Material::CreateSimpleColor()
{
	auto mat = new ::Material(1);
	mat->SetPass(0, new ::Pass("default", "simple.color"));
	return mat;
}

bool Shader::Create(const std::string& vsCode, uint32_t vcbLength, const std::string& psCode, uint32_t pcbLength)
{
	auto vsCodeLength = vsCode.size();
	auto psCodeLength = psCode.size();

	autor<ID3DBlob> vsBlob = nullptr;
	autor<ID3DBlob> psBlob = nullptr;
	autor<ID3DBlob> errorBlob = nullptr;

	autor<ID3D11InputLayout> shaderLayout = nullptr;
	autor<ID3D11VertexShader>  vertexShader = nullptr;
	autor<ID3D11PixelShader> pixelShader = nullptr;
	autor<rhi::Buffer> vertexConstBuffer = nullptr;
	autor<rhi::Buffer> pixelConstBuffer = nullptr;

	//compile shader
	auto ret = ::D3DCompile(
		vsCode.c_str(), vsCodeLength,
		NULL, NULL, NULL,
		"VSMain", "vs_5_0",
		0, 0,
		&vsBlob.pointer, &errorBlob.pointer);

	if (S_OK != ret)
	{
		const char* reason = (const char*)errorBlob->GetBufferPointer();
		ENSURE(false);
	}

	ret = ::D3DCompile(
		psCode.c_str(), psCodeLength,
		NULL, NULL, NULL,
		"PSMain", "ps_5_0",
		0, 0,
		&psBlob.pointer, &errorBlob.pointer);

	if (S_OK != ret)
	{
		const char* reason = (const char*)errorBlob->GetBufferPointer();
		ENSURE(false);
	}

	// create shader
	ret = GetRenderSystem()->GetDevice()->GetRaw()->CreateVertexShader(
		vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(),
		NULL,
		&(vertexShader.pointer));

	if (S_OK != ret)
		return false;

	ret = GetRenderSystem()->GetDevice()->GetRaw()->CreatePixelShader(
		psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(),
		NULL,
		&(pixelShader.pointer));

	if (S_OK != ret)
		return false;

	D3D11_INPUT_ELEMENT_DESC layoutDesc[3];
	::ZeroMemory(layoutDesc, sizeof(layoutDesc));

	layoutDesc[0].SemanticName = "POSITION";
	layoutDesc[0].Format = DXGI_FORMAT_R32G32_FLOAT;
	layoutDesc[0].AlignedByteOffset = 0;
	layoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	layoutDesc[1].SemanticName = "TEXCOORD";
	layoutDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	layoutDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	layoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	layoutDesc[2].SemanticName = "COLOR";
	layoutDesc[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	layoutDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	layoutDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	ret = GetRenderSystem()->GetDevice()->GetRaw()->CreateInputLayout(
		layoutDesc, sizeof(layoutDesc) / sizeof(layoutDesc[0]),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
		&(shaderLayout.pointer));

	if (S_OK != ret)
		return false;

	D3D11_BUFFER_DESC cbDesc =
	{
		0,							//UINT ByteWidth;
		D3D11_USAGE_DYNAMIC,		//D3D11_USAGE Usage;
		D3D11_BIND_CONSTANT_BUFFER,	//UINT BindFlags;
		D3D11_CPU_ACCESS_WRITE,		//UINT CPUAccessFlags;
		0,							//UINT MiscFlags;
		0							//UINT StructureByteStride;
	};
	
	if (vcbLength > 0)
	{
		vertexConstBuffer = GetRenderSystem()->GetDevice()->CreateBuffer(
			rhi::BufferBinding::Constant,
			rhi::ResourceUsage::Dynamic,
			vcbLength
		);
		if (vertexConstBuffer.is_null())
			return false;

	}

	if (pcbLength > 0)
	{
		pixelConstBuffer = GetRenderSystem()->GetDevice()->CreateBuffer(
			rhi::BufferBinding::Constant,
			rhi::ResourceUsage::Dynamic,
			pcbLength
		);
		if (pixelConstBuffer.is_null())
			return false;
	}

	m_pixelShader = std::move(pixelShader);
	m_vertexShader = std::move(vertexShader);
	m_shaderLayout = std::move(shaderLayout);
	m_vertexConstBuffer = std::move(vertexConstBuffer);
	m_pixelConstBuffer = std::move(pixelConstBuffer);
	return true;
}

void Shader::Destroy()
{
	m_pixelShader.release();
	m_vertexShader.release();
	m_shaderLayout.release();
	m_vertexConstBuffer.release();
	m_pixelConstBuffer.release();
}

class DefaultVSData : public VSData
{
public:
	virtual const char* GetName() override { return "default"; }
	virtual const char* GetCode() override
	{
		return R"(
			cbuffer scene
			{
				float4x2 matrixView;
				float4x4 matrixProj;
			}
			struct GeometryVertex
			{
				float2 position : POSITION;
				float2 texcoord : TEXCOORD0;
				float4 vtxcolor : COLOR;
			};
			struct VertexOutput
			{
				float4 position : SV_POSITION;
				float2 texcoord : TEXCOORD0;
				float4 vtxcolor : COLOR;
			};
			VertexOutput VSMain(GeometryVertex input)
			{
				VertexOutput output;
				float3 position = float3(input.position,1);
				float2 viewPos = float2(
					dot(position, float3(matrixView[0][0],matrixView[1][0],matrixView[2][0])),
					dot(position, float3(matrixView[0][1],matrixView[1][1],matrixView[2][1])));
				output.position = mul(float4(viewPos, 0, 1), matrixProj);
				output.texcoord = input.texcoord;
				output.vtxcolor = input.vtxcolor;
				return output;
			}
		)";
	}
	virtual uint32_t GetConstBufferLength() override { return 0; }
};

class SimpleColorPSData : public PSData
{
	virtual const char* GetName() override { return "simple.color"; }
	virtual const char* GetCode() override
	{
		return R"(
			struct VertexInput
			{
				float4 position : SV_POSITION;
				float2 texcoord : TEXCOORD0;
				float4 vtxcolor : COLOR;
			};
			float4 PSMain(VertexInput input):SV_TARGET
			{
				return input.vtxcolor;
			}
		)";
	}
	virtual uint32_t GetConstBufferLength() override { return 0; }
};

class SimpleTexturePSData : public PSData
{
	virtual const char* GetName() override { return "simple.texture"; }
	virtual const char* GetCode() override
	{
		return R"(
			Texture2D Tex;
			SamplerState State;
			struct VertexInput
			{
				float4 position : SV_POSITION;
				float2 texcoord : TEXCOORD0;
				float4 vtxcolor : COLOR;
			};
			float4 PSMain(VertexInput input):SV_TARGET
			{
				return Tex.Sample(State, input.texcoord);
			}
		)";
	}
	virtual uint32_t GetConstBufferLength() override { return 0; }
};

class ColorTexturePSData : public PSData
{
	virtual const char* GetName() override { return "color.texture"; }
	virtual const char* GetCode() override
	{
		return R"(
			Texture2D Tex;
			SamplerState State;
			struct VertexInput
			{
				float4 position : SV_POSITION;
				float2 texcoord : TEXCOORD0;
				float4 vtxcolor : COLOR;
			};
			float4 PSMain(VertexInput input):SV_TARGET
			{
				return input.vtxcolor * Tex.Sample(State, input.texcoord);
			}
		)";
	}
	virtual uint32_t GetConstBufferLength() override { return 0; }
};

ShaderLib::ShaderLib()
{
	VSData* vsd = new DefaultVSData();
	m_vsSources[vsd->GetName()] = vsd;

	PSData* psd;
	psd = new SimpleColorPSData();
	m_psSources[psd->GetName()] = psd;

	psd = new SimpleTexturePSData();
	m_psSources[psd->GetName()] = psd;

	psd = new ColorTexturePSData();
	m_psSources[psd->GetName()] = psd;
}

ShaderLib::~ShaderLib()
{
	for (auto& psd : m_psSources)
	{
		delete psd.second;
	}
	m_psSources.clear();

	for (auto& vsd : m_vsSources)
	{
		delete vsd.second;
	}
	m_vsSources.clear();
}

std::string ShaderLib::GetEffectName(const std::string& vsName, const std::string& psName)
{
	return std::move(vsName + psName);
}

Shader* ShaderLib::GetShaderByName(const std::string& vsName, const std::string& psName)
{
	std::string effectName = GetEffectName(vsName, psName);
	if (!m_shaders.count(effectName))
	{
		if (!BuildShader(effectName, vsName, psName))
		{
			return false;
		}
	}
	return m_shaders[effectName];
}

bool ShaderLib::BuildShader(const std::string& effectName, const std::string& vsName, const std::string& psName)
{
	auto vsData = m_vsSources[vsName];
	auto psData = m_psSources[psName];
	if (vsData == nullptr || psData == nullptr)
		return false;

	Shader* shader = new Shader();
	if (shader->Create(
		vsData->GetCode(), vsData->GetConstBufferLength(),
		psData->GetCode(), psData->GetConstBufferLength()))
	{
		m_shaders[effectName] = shader;
		return true;
	}
	delete shader;
	return false;
}

Pass::Pass(const Pass& other)
	: m_vsName(other.m_vsName)
	, m_psName(other.m_psName)
	, m_textures(other.m_textures.size())
	, m_vsConstants(other.m_vsConstants.size())
	, m_psConstants(other.m_psConstants.size())
	, m_blendMode(other.m_blendMode)
{
	for (size_t i = 0, n = m_textures.size(); i < n; i++)
	{
		m_textures[i] = other.m_textures[i];
		m_textures[i]->AddRef();
	}

	if (other.GetVSConstantLength() > 0)
	{
		memcpy(&(m_vsConstants[0]), &(other.m_vsConstants[0]), other.GetVSConstantLength());
	}
	if (other.GetPSConstantLength() > 0)
	{
		memcpy(&(m_psConstants[0]), &(other.m_psConstants[0]), other.GetPSConstantLength());
	}
}

Pass::~Pass()
{
	for (auto& t : m_textures)
	{
		t->Release();
	}
	m_textures.clear();
}

Pass* Pass::Clone()
{
	Pass* p = new Pass(*this);
	return p;
}

bool Pass::IsSame(g2d::Pass* other) const
{
	ENSURE(other != nullptr);
	if (!IsSameType(other))
		return false;

	Pass* p = reinterpret_cast<Pass*>(&other);

	if (this == p)
		return true;

	if (m_blendMode != p->m_blendMode ||
		m_vsName != p->m_vsName ||
		m_psName != p->m_psName)
		return false;

	if (m_textures.size() != p->m_textures.size() ||
		m_vsConstants.size() != p->m_vsConstants.size() ||
		m_psConstants.size() != p->m_psConstants.size())
	{
		return false;
	}

	for (size_t i = 0, n = m_textures.size(); i < n; i++)
	{
		if (!m_textures[i]->IsSame(p->m_textures[i]))
		{
			return false;
		}
	}

	//we have no idea how to deal with floats.
	if (m_vsConstants.size() > 0 &&
		0 != memcmp(&(m_vsConstants[0]), &(p->m_vsConstants[0]), m_vsConstants.size() * sizeof(float)))
	{
		return  false;
	}

	if (m_psConstants.size() > 0 &&
		0 != memcmp(&(m_psConstants[0]), &(p->m_psConstants[0]), m_psConstants.size() * sizeof(float)))
	{
		return false;
	}
	return true;
}

void Pass::SetTexture(uint32_t index, g2d::Texture* tex, bool autoRelease)
{
	size_t size = m_textures.size();
	if (index >= size)
	{
		m_textures.resize(index + 1);
		for (size_t i = size; i < index; i++)
		{
			m_textures[i] = nullptr;
		}
	}

	if (m_textures[index])
	{
		m_textures[index]->Release();
	}
	m_textures[index] = tex;
	if (!autoRelease)
	{
		m_textures[index]->AddRef();
	}
}

void Pass::SetVSConstant(uint32_t index, float* data, uint32_t size, uint32_t count)
{
	if (count == 0)
		return;

	if (index + count > m_vsConstants.size())
	{
		m_vsConstants.resize(index + count);
	}

	for (uint32_t i = 0; i < count; i++)
	{
		memcpy(&(m_vsConstants[index + i]), data + i*size, size);
	}
}

void Pass::SetPSConstant(uint32_t index, float* data, uint32_t size, uint32_t count)
{
	if (count == 0)
		return;

	if (index + count > m_psConstants.size())
	{
		m_psConstants.resize(index + count);
	}

	for (uint32_t i = 0; i < count; i++)
	{
		memcpy(&(m_psConstants[index + i]), data + i*size, size);
	}
}

Material::Material(uint32_t passCount)
	: m_passes(passCount)
{

}

Material::Material(const Material& other)
	: m_passes(other.m_passes.size())
{
	for (size_t i = 0, n = m_passes.size(); i < n; i++)
	{
		m_passes[i] = other.m_passes[i]->Clone();
	}
}

void Material::SetPass(uint32_t index, Pass* p)
{
	ENSURE(index < m_passes.size());
	m_passes[index] = p;
}

Material::~Material()
{
	for (auto& p : m_passes)
	{
		p->Release();
	}
	m_passes.clear();
}

g2d::Pass* Material::GetPassByIndex(uint32_t index) const
{
	ENSURE(index < m_passes.size());
	return m_passes[index];
}

uint32_t Material::GetPassCount() const
{
	return static_cast<uint32_t>(m_passes.size());
}

bool Material::IsSame(g2d::Material* other) const
{
	ENSURE(other != nullptr);

	if (this == other)
		return true;

	if (!IsSameType(other))
		return false;

	if (other->GetPassCount() != GetPassCount())
		return false;

	for (uint32_t i = 0; i < GetPassCount(); i++)
	{
		if (!m_passes[i]->IsSame(other->GetPassByIndex(i)))
		{
			return false;
		}
	}
	return true;
}

g2d::Material* Material::Clone() const
{
	Material* newMat = new Material(*this);
	return newMat;
}

void Material::Release()
{
	delete this;
}
