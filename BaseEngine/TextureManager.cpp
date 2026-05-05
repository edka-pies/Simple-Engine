#include "TextureManager.h"

TextureManager::~TextureManager() {
    Clear();
}

TextureManager& TextureManager::GetInstance()
{
	static TextureManager instance;
    return instance;
}

std::shared_ptr<Texture> TextureManager::GetTexture(const std::string& path) {
    auto it = m_Textures.find(path);

    if (it != m_Textures.end()) {
        return it->second;
    }

    auto newTexture = std::make_shared<Texture>();

    newTexture->Init(path.c_str());

    m_Textures[path] = newTexture;

    return newTexture;
}

void TextureManager::Clear() {
    m_Textures.clear();
}