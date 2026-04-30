#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "Texture.h"

class TextureManager {
public:
    ~TextureManager();

    static TextureManager& GetInstance();

    // Returns a shared pointer to the texture. Loads it if it doesn't exist.
    std::shared_ptr<Texture> GetTexture(const std::string& path);

    // Frees all textures from the map manually if needed
    void Clear();

private:
    TextureManager() = default;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_Textures;
};