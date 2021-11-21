#include "RenderConfigs.hpp"

#include "AssetManager.hpp"

Gbuffer::Gbuffer(size_t geomW, size_t geomH, size_t lightW, size_t lightH) {
    auto gal = Gal::GetImplementation();

    auto colorConf = gal->CreateTexture2DConfig(geomW, geomH, Gal::Format::R8G8B8A8);
    color = gal->BuildTexture(colorConf);

    auto normalConf = gal->CreateTexture2DConfig(geomW, geomH, Gal::Format::R8G8B8A8);
    normal = gal->BuildTexture(normalConf);

    auto dsConf = gal->CreateTexture2DConfig(geomW, geomH, Gal::Format::D24S8);
    depthStencil = gal->BuildTexture(dsConf);

    auto geomFbConf = gal->CreateFramebufferConfig();
    geomFbConf->SetTexture(0, color);
    geomFbConf->SetTexture(1, normal);
    geomFbConf->SetDepthStencil(depthStencil);

    geomFramebuffer = gal->BuildFramebuffer(geomFbConf);
    geomFramebuffer->SetViewport(0, 0, geomW, geomH);

    auto finalColorConf = gal->CreateTexture2DConfig(lightW, lightH, Gal::Format::R8G8B8A8);
    finalColor = gal->BuildTexture(finalColorConf);

    auto lightFbConf = gal->CreateFramebufferConfig();
    lightFbConf->SetTexture(0, finalColor);

    lightFramebuffer = gal->BuildFramebuffer(lightFbConf);
    lightFramebuffer->SetViewport(0, 0, lightW, lightH);

    std::string vertexSource, pixelSource;
    {
        auto vertAsset = AssetManager::GetAssetByAssetName("/altcraft/shaders/vert/light");
        vertexSource = std::string((char*)vertAsset->data.data(), (char*)vertAsset->data.data() + vertAsset->data.size());

        auto pixelAsset = AssetManager::GetAssetByAssetName("/altcraft/shaders/frag/light");
        pixelSource = std::string((char*)pixelAsset->data.data(), (char*)pixelAsset->data.data() + pixelAsset->data.size());
    }

    auto lightPPC = gal->CreatePipelineConfig();
    lightPPC->SetTarget(lightFramebuffer);
    lightPPC->AddStaticTexture("color", color);
    lightPPC->AddStaticTexture("normal", normal);
    lightPPC->AddStaticTexture("depthStencil", depthStencil);

    lightPPC->SetVertexShader(gal->LoadVertexShader(vertexSource));
    lightPPC->SetPixelShader(gal->LoadPixelShader(pixelSource));

    auto lightBB = lightPPC->BindVertexBuffer({
        {"pos", Gal::Type::Vec2},
        {"uvPos", Gal::Type::Vec2}
        });

    lightPipeline = gal->BuildPipeline(lightPPC);

    constexpr float quadVertices[] = {
        // pos         // uv
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    lightBuffer = gal->CreateBuffer();
    lightBuffer->SetData({ reinterpret_cast<const std::byte*>(quadVertices), reinterpret_cast<const std::byte*>(quadVertices) + sizeof(quadVertices) });

    lightPipelineInstance = lightPipeline->CreateInstance({
        {lightBB, lightBuffer}
        });
}
