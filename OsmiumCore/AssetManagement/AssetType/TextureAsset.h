//
// Created by Shadow on 5/5/2025.
//

#ifndef TEXTUREASSET_H
#define TEXTUREASSET_H

#include "../Asset.h"

class TextureAsset : public Asset{
protected:
    ~TextureAsset() = default;

    void Load_Impl() override;

    void Unload_Impl(bool immediate) override;

public:
    [[nodiscard]]auto GetRessourceMutex() -> std::mutex & override;
    [[nodiscard]] auto GetTextureHandle() const -> unsigned long;
    explicit TextureAsset(const xg::Guid &id);
#ifdef EDITOR
    TextureAsset(const xg::Guid &id, const std::string &filename);
#endif
private:
    unsigned long textureHandle = 0;
};



#endif //TEXTUREASSET_H
