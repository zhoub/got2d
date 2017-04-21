#include "engine.h"
#include "render_system.h"

g2d::Texture* g2d::Texture::LoadFromFile(const char* path)
{
	std::string resourcePath = ::GetEngineImpl()->GetResourceRoot() + path;
	return ::GetRenderSystem()->CreateTextureFromFile(resourcePath.c_str());
}

Texture::Texture(std::string resPath)
	: m_resPath(std::move(resPath))
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

void UploadImageToTexture(rhi::Texture2D* texture, uint8_t* data, bool hasAlpha)
{
	auto mappedResouce = GetRenderSystem()->GetContext()->Map(texture);
	if (mappedResouce.success)
	{
		uint8_t* colorBuffer = static_cast<uint8_t*>(mappedResouce.data);
		if (hasAlpha)
		{
			auto srcPitch = texture->GetWidth() * 4;
			auto height = texture->GetHeight();
			for (uint32_t i = 0; i < height; i++)
			{
				auto dstPtr = colorBuffer + i * mappedResouce.linePitch;
				auto srcPtr = data + i * srcPitch;
				memcpy(dstPtr, srcPtr, srcPitch);
			}
		}
		else
		{

			auto width = texture->GetWidth();
			auto height = texture->GetHeight();
			auto srcPitch = width * 3;
			for (uint32_t i = 0; i < height; i++)
			{
				auto dstPtr = colorBuffer + i * mappedResouce.linePitch;
				auto srcPtr = data + i * srcPitch;

				for (uint32_t j = 0; j < width; j++)
				{
					memcpy(dstPtr + j * 4, srcPtr + j * 3, 3);
					dstPtr[3 + j * 4] = 255;
				}
			}
		}

		GetRenderSystem()->GetContext()->Unmap(texture);
		GetRenderSystem()->GetContext()->GenerateMipmaps(texture);
	}
}

#include "engine.h"
#include <img/file_data.h>
#include <img/img_data.h>

bool TexturePool::CreateDefaultTexture()
{
	m_defaultTexture = GetRenderSystem()->GetDevice()->CreateTexture2D(
		rhi::TextureFormat::RGBA,
		rhi::ResourceUsage::Dynamic,
		rhi::TextureBinding::ShaderResource,
		2, 2);

	if (m_defaultTexture != nullptr)
	{
		uint8_t boardData[] =
		{
			0,0,0,255,255,255,
			255,255,255,0,0,0
		};
		UploadImageToTexture(m_defaultTexture, boardData, false);
		return true;
	}
	else
	{
		return false;
	}
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
		auto tex = GetRenderSystem()->GetDevice()->CreateTexture2D(
			rhi::TextureFormat::RGBA,
			rhi::ResourceUsage::Dynamic,
			rhi::TextureBinding::ShaderResource,
			img.width, img.height);
		if (tex != nullptr)
		{
			UploadImageToTexture(tex, img.raw_data, img.has_alpha);
			m_textures[resourcePath] = tex;
		}

		destroy_img_data(img);
		return result;
	}
	return false;
}

void TexturePool::Destroy()
{
	m_defaultTexture.release();
	for (auto& t : m_textures)
	{
		t.second->Release();
	}
	m_textures.clear();
}

rhi::Texture2D* TexturePool::GetTexture(const std::string& resource)
{
	if (m_textures.count(resource) == 0)
	{
		if (!LoadTextureFromFile(resource))
		{
			return m_defaultTexture;
		}
	}

	return m_textures.at(resource);
}
