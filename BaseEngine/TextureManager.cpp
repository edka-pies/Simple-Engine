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
    // 1. Look for the texture in our map
    auto it = m_Textures.find(path);

    // 2. If found, return it immediately
    if (it != m_Textures.end()) {
        return it->second;
    }

    // 3. If not found, create it
    // Using make_shared is more efficient than "new Texture()"
    auto newTexture = std::make_shared<Texture>();

    // Call your Init function
    newTexture->Init(path.c_str());

    // 4. Store it in the map for next time
    m_Textures[path] = newTexture;

    return newTexture;
}

void TextureManager::Clear() {
    // Because we use shared_ptr, clearing the map will trigger 
    // the Texture destructor (and glDeleteTextures) automatically 
    // if no one else is holding a pointer to them.
    m_Textures.clear();
}