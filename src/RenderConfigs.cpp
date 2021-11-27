#include "RenderConfigs.hpp"

#include "AssetManager.hpp"

std::string LoadShaderCode(std::string_view assetPath) {
    auto gal = Gal::GetImplementation();
    auto codeAsset = AssetManager::GetAssetByAssetName(std::string(assetPath));
    auto codePtr = reinterpret_cast<const char*>(codeAsset->data.data());
    return std::string(codePtr, codePtr + codeAsset->data.size());
}

std::shared_ptr<Gal::Shader> LoadVertexShader(std::string_view assetPath) {
    auto gal = Gal::GetImplementation();
    return gal->LoadVertexShader(LoadShaderCode(assetPath));
}

std::shared_ptr<Gal::Shader> LoadPixelShader(std::string_view assetPath) {
    auto gal = Gal::GetImplementation();
    return gal->LoadPixelShader(LoadShaderCode(assetPath));
}

PostProcess::PostProcess(
    std::shared_ptr<Gal::Shader> pixelShader,
    std::vector<std::pair<std::string_view, std::shared_ptr<Gal::Texture>>> inputTextures,
    std::vector<std::pair<std::string_view, Gal::Type>> inputParameters,
    size_t width,
    size_t height,
    Gal::Format format,
    Gal::Filtering filtering) {
    auto gal = Gal::GetImplementation();

    auto texConf = gal->CreateTexture2DConfig(width, height, format);
    texConf->SetMinFilter(filtering);
    texConf->SetMaxFilter(filtering);

    result = gal->BuildTexture(texConf);

    auto fbConf = gal->CreateFramebufferConfig();
    fbConf->SetTexture(0, result);

    framebuffer = gal->BuildFramebuffer(fbConf);
    framebuffer->SetViewport(0, 0, width, height);

    auto fbPPC = gal->CreatePipelineConfig();
    fbPPC->SetTarget(framebuffer);
    for (auto&& [name, texture] : inputTextures) {
        fbPPC->AddStaticTexture(name, texture);
    }
    for (auto&& [name, type] : inputParameters) {
        fbPPC->AddShaderParameter(name, type);
    }
    fbPPC->SetVertexShader(LoadVertexShader("/altcraft/shaders/vert/pp"));
    fbPPC->SetPixelShader(pixelShader);
    auto fbBufferBB = fbPPC->BindVertexBuffer({
        {"pos", Gal::Type::Vec2},
        {"uvPos", Gal::Type::Vec2}
        });

    pipeline = gal->BuildPipeline(fbPPC);

    fbBuffer = gal->CreateBuffer();
    constexpr float quadVertices[] = {
        // pos         // uv
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    auto quadPtr = reinterpret_cast<const std::byte*>(quadVertices);
    fbBuffer->SetData({ quadPtr, quadPtr + sizeof(quadVertices) });

    pipelineInstance = pipeline->CreateInstance({
        {fbBufferBB, fbBuffer}
        });
}

Gbuffer::Gbuffer(size_t geomW, size_t geomH, size_t lightW, size_t lightH) {
    auto gal = Gal::GetImplementation();

    auto colorConf = gal->CreateTexture2DConfig(geomW, geomH, Gal::Format::R8G8B8);
    colorConf->SetMinFilter(Gal::Filtering::Bilinear);
    colorConf->SetMaxFilter(Gal::Filtering::Bilinear);
    color = gal->BuildTexture(colorConf);

    auto normalConf = gal->CreateTexture2DConfig(geomW, geomH, Gal::Format::R8G8B8);
    normalConf->SetMinFilter(Gal::Filtering::Bilinear);
    normalConf->SetMaxFilter(Gal::Filtering::Bilinear);
    normal = gal->BuildTexture(normalConf);

    auto addColorConf = gal->CreateTexture2DConfig(geomW, geomH, Gal::Format::R8G8B8);
    addColorConf->SetMinFilter(Gal::Filtering::Bilinear);
    addColorConf->SetMaxFilter(Gal::Filtering::Bilinear);
    addColor = gal->BuildTexture(addColorConf);

    auto lightConf = gal->CreateTexture2DConfig(geomW, geomH, Gal::Format::R8G8B8);
    lightConf->SetMinFilter(Gal::Filtering::Bilinear);
    lightConf->SetMaxFilter(Gal::Filtering::Bilinear);
    light = gal->BuildTexture(lightConf);

    auto dsConf = gal->CreateTexture2DConfig(geomW, geomH, Gal::Format::D24S8);
    dsConf->SetMinFilter(Gal::Filtering::Bilinear);
    dsConf->SetMaxFilter(Gal::Filtering::Bilinear);
    depthStencil = gal->BuildTexture(dsConf);

    auto geomFbConf = gal->CreateFramebufferConfig();
    geomFbConf->SetTexture(0, color);
    geomFbConf->SetTexture(1, normal);
    geomFbConf->SetTexture(2, addColor);
    geomFbConf->SetTexture(3, light);
    geomFbConf->SetDepthStencil(depthStencil);

    geomFramebuffer = gal->BuildFramebuffer(geomFbConf);
    geomFramebuffer->SetViewport(0, 0, geomW, geomH);

    std::vector<std::pair<std::string_view, std::shared_ptr<Gal::Texture>>> lightingTexture = {
        {"color", color},
        {"addColor", addColor},
        {"normal", normal},
        {"light", light},
        {"depthStencil", depthStencil},
    };

    std::vector<std::pair<std::string_view, Gal::Type>> lightingParameters = {
        {"renderBuff", Gal::Type::Int32},
    };

    lightingPass = std::make_unique<PostProcess>(LoadPixelShader("/altcraft/shaders/frag/light"),
        lightingTexture, 
        lightingParameters,
        lightW,
        lightH,
        Gal::Format::R8G8B8A8,
        Gal::Filtering::Bilinear);
}
