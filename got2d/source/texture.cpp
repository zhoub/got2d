#include "render_system.h"

g2d::Texture::~Texture() {}

Texture::Texture(const char* resPath) : m_resPath(resPath)
{

}
void Texture::Release()
{
	delete this;
}