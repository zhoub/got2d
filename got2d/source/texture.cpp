#include "engine.h"
#include "render_system.h"

g2d::Texture* g2d::Texture::LoadFromFile(const char* path)
{
	std::string resourcePath = ::GetEngineImpl()->GetResourceRoot() + path;
	return ::GetRenderSystem()->CreateTextureFromFile(resourcePath.c_str());
}

Texture::Texture(std::string resPath) : m_resPath(std::move(resPath))
{

}

bool Texture::IsSame(g2d::Texture* other) const
{
	ENSURE(other != nullptr);
	if (this == other)
		return true;

	if (!same_type(other, this))
		return false;

	auto timpl = reinterpret_cast<::Texture*>(other);
	return timpl->m_resPath == m_resPath;
}

void Texture::AddRef()
{
	m_refCount++;
}

void Texture::Release()
{
	if (--m_refCount == 0)
	{
		delete this;
	}
}

bool Texture2D::Create(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0)
		return false;

	autor<rhi::Texture2D> texturePtr = nullptr;
	autor<rhi::ShaderResourceView> shaderResourceViewPtr = nullptr;

	texturePtr = GetRenderSystem()->GetDevice()->CreateTexture2D(
		rhi::TextureFormat::RGBA,
		rhi::ResourceUsage::Dynamic,
		rhi::TextureBinding::ShaderResource,
		width, height);
	if (texturePtr.is_null())
	{
		return false;
	}

	shaderResourceViewPtr = GetRenderSystem()->GetDevice()->CreateShaderResourceView(texturePtr);
	if (shaderResourceViewPtr.is_null())
	{
		return false;
	}

	m_texture = std::move(texturePtr);
	m_shaderView = std::move(shaderResourceViewPtr);
	m_width = width;
	m_height = height;
	return true;
}

void Texture2D::UploadImage(uint8_t* data, bool hasAlpha)
{
	auto mappedResouce = GetRenderSystem()->GetContext()->Map(m_texture);
	if (mappedResouce.success)
	{
		uint8_t* colorBuffer = static_cast<uint8_t*>(mappedResouce.data);
		if (hasAlpha)
		{
			int srcPitch = m_width * 4;
			for (uint32_t i = 0; i < m_height; i++)
			{
				auto dstPtr = colorBuffer + i * mappedResouce.linePitch;
				auto srcPtr = data + i * srcPitch;
				memcpy(dstPtr, srcPtr, srcPitch);
			}
		}
		else
		{
			int srcPitch = m_width * 3;
			for (uint32_t i = 0; i < m_height; i++)
			{
				auto dstPtr = colorBuffer + i * mappedResouce.linePitch;
				auto srcPtr = data + i * srcPitch;
				for (uint32_t j = 0; j < m_width; j++)
				{
					memcpy(dstPtr + j * 4, srcPtr + j * 3, 3);
					dstPtr[3 + j * 4] = 255;
				}
			}
		}

		GetRenderSystem()->GetContext()->Unmap(m_texture);
		GetRenderSystem()->GetContext()->GenerateMipmaps(m_shaderView);
	}
}

void Texture2D::Destroy()
{
	m_texture.release();
	m_shaderView.release();
	m_width = 0;
	m_height = 0;
}

#include "engine.h"
#include <img/file_data.h>
#include <img/img_data.h>

bool TexturePool::CreateDefaultTexture()
{
	if (m_defaultTexture.Create(2, 2))
	{
		uint8_t boardData[] =
		{
			0,0,0,255,255,255,
			255,255,255,0,0,0
		};
		m_defaultTexture.UploadImage(boardData, false);
		m_textures.insert(std::make_pair<>("", &m_defaultTexture));
		return true;
	}
	m_defaultTexture.Destroy();
	return false;
}

bool TexturePool::LoadTextureFromFile(std::string resourcePath)
{
	file_data f;
	if (!load_file(resourcePath.c_str(), f))
		return false;

	img_data img;
	auto result = true;

	result = read_image(f.buffer, img);
	destroy_file_data(f);

	if (result)
	{
		auto tex = new ::Texture2D();
		result = tex->Create(img.width, img.height);
		if (result)
		{
			tex->UploadImage(img.raw_data, img.has_alpha);
			m_textures[resourcePath] = tex;
		}
		destroy_img_data(img);
		return result;
	}
	return false;
}

void TexturePool::Destroy()
{
	m_textures.erase("");
	m_defaultTexture.Destroy();
	for (auto& t : m_textures)
	{
		t.second->Destroy();
		delete t.second;
	}
	m_textures.clear();
}

Texture2D* TexturePool::GetTexture(const std::string& resource)
{
	if (m_textures.count(resource) == 0)
	{
		if (!LoadTextureFromFile(resource))
		{
			return nullptr;
		}
	}

	return m_textures.at(resource);
}
