#include "render_system.h"
#include <d3dcompiler.h>
#pragma comment(lib,"d3dcompiler.lib")

bool Shader::Create(ID3D11Device* device, const char* vsCode, const char* psCode)
{
	auto vsCodeLength = strlen(vsCode) + 1;
	auto psCodeLength = strlen(psCode) + 1;

	ID3DBlob* vsBlob = nullptr;
	ID3DBlob* psBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	do
	{

		//compile shader
		auto ret = ::D3DCompile(
			vsCode, vsCodeLength,
			NULL, NULL, NULL,
			"VSMain", "vs_5_0",
			0, 0,
			&vsBlob, &errorBlob);

		if (S_OK != ret)
		{
			//errorBlob->GetBufferPointer();
			errorBlob->Release();
			errorBlob = nullptr;
			break;
		}

		ret = ::D3DCompile(
			psCode, psCodeLength,
			NULL, NULL, NULL,
			"PSMain", "ps_5_0",
			0, 0,
			&psBlob, &errorBlob);

		if (S_OK != ret)
		{
			//errorBlob->GetBufferPointer();
			errorBlob->Release();
			errorBlob = nullptr;
			break;
		}

		// create shader
		ret = device->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			NULL,
			&m_vertexShader);

		if (S_OK != ret)
			break;

		ret = device->CreatePixelShader(
			psBlob->GetBufferPointer(),
			psBlob->GetBufferSize(),
			NULL,
			&m_pixelShader);

		if (S_OK != ret)
			break;

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

		ret = device->CreateInputLayout(
			layoutDesc, sizeof(layoutDesc) / sizeof(layoutDesc[0]),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			&m_shaderLayout);

		if (S_OK != ret)
			break;

		SR(vsBlob);
		SR(psBlob);
		return true;
	} while (false);

	SR(vsBlob);
	SR(psBlob);
	Destroy();
	return false;
}
void Shader::Destroy()
{
	SR(m_vertexShader);
	SR(m_pixelShader);
	SR(m_shaderLayout);
}

ID3D11VertexShader* Shader::GetVertexShader() { return m_vertexShader; }
ID3D11PixelShader* Shader::GetPixelShader() { return m_pixelShader; }
ID3D11InputLayout* Shader::GetInputLayout() { return m_shaderLayout; }