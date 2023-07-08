#include "RenderConfigs.hpp"

#include <chrono>
#include <random>

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

TextureFbCopy::TextureFbCopy(
    std::shared_ptr<Gal::Texture> inputTexture,
    std::shared_ptr<Gal::Texture> outputTexture,
    std::shared_ptr<Gal::Shader> copyShader) {

    auto gal = Gal::GetImplementation();

    auto fbConf = gal->CreateFramebufferConfig();
    fbConf->SetTexture(0, outputTexture);

    auto [outputW, outputH, outputD] = outputTexture->GetSize();

    framebuffer = gal->BuildFramebuffer(fbConf);
    framebuffer->SetViewport(0, 0, outputW, outputH);

    auto fbPPC = gal->CreatePipelineConfig();
    fbPPC->SetTarget(framebuffer);

    fbPPC->AddStaticTexture("inputTexture", inputTexture);

    fbPPC->SetVertexShader(LoadVertexShader("/altcraft/shaders/vert/quad"));
    fbPPC->SetPixelShader(copyShader ? copyShader : LoadPixelShader("/altcraft/shaders/frag/copy"));
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

TextureFbCopy::TextureFbCopy(
    std::shared_ptr<Gal::Texture> inputTexture,
    std::shared_ptr<Gal::Framebuffer> outputFb,
    std::shared_ptr<Gal::Shader> copyShader) {

    auto gal = Gal::GetImplementation();

    framebuffer = std::move(outputFb);

    auto fbPPC = gal->CreatePipelineConfig();
    fbPPC->SetTarget(framebuffer);

    fbPPC->AddStaticTexture("inputTexture", inputTexture);

    fbPPC->SetVertexShader(LoadVertexShader("/altcraft/shaders/vert/quad"));
    fbPPC->SetPixelShader(copyShader ? copyShader : LoadPixelShader("/altcraft/shaders/frag/copy"));
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
    fbPPC->SetVertexShader(LoadVertexShader("/altcraft/shaders/vert/quad"));
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

Gbuffer::Gbuffer(size_t geomW, size_t geomH, size_t lightW, size_t lightH, int ssaoSamples, size_t ssaoW, size_t ssaoH) {
    auto gal = Gal::GetImplementation();

    auto colorConf = gal->CreateTexture2DConfig(geomW, geomH, Gal::Format::R8G8B8);
    colorConf->SetMinFilter(Gal::Filtering::Bilinear);
    colorConf->SetMaxFilter(Gal::Filtering::Bilinear);
    color = gal->BuildTexture(colorConf);

    auto normalConf = gal->CreateTexture2DConfig(geomW, geomH, Gal::Format::R8G8B8SN);
    normalConf->SetMinFilter(Gal::Filtering::Bilinear);
    normalConf->SetMaxFilter(Gal::Filtering::Bilinear);
    normal = gal->BuildTexture(normalConf);

    auto lightConf = gal->CreateTexture2DConfig(geomW, geomH, Gal::Format::R8G8B8);
    lightConf->SetMinFilter(Gal::Filtering::Bilinear);
    lightConf->SetMaxFilter(Gal::Filtering::Bilinear);
    light = gal->BuildTexture(lightConf);

    auto dsConf = gal->CreateTexture2DConfig(geomW, geomH, Gal::Format::D24S8);
    dsConf->SetMinFilter(Gal::Filtering::Bilinear);
    dsConf->SetMaxFilter(Gal::Filtering::Bilinear);
    depthStencil = gal->BuildTexture(dsConf);

    auto geomFbConf = gal->CreateFramebufferConfig();
    geomFbConf->SetDepthStencil(depthStencil);
    geomFbConf->SetTexture(0, color);
    geomFbConf->SetTexture(1, normal);
    geomFbConf->SetTexture(2, light);

    geomFramebuffer = gal->BuildFramebuffer(geomFbConf);
    geomFramebuffer->SetViewport(0, 0, geomW, geomH);

    if (ssaoSamples > 0) {
        auto noiseConf = gal->CreateTexture2DConfig(4, 4, Gal::Format::R8G8B8SN);
        noiseConf->SetWrapping(Gal::Wrapping::Repeat);
        noiseConf->SetMinFilter(Gal::Filtering::Bilinear);
        noiseConf->SetMaxFilter(Gal::Filtering::Bilinear);
        ssaoNoise = gal->BuildTexture(noiseConf);

        std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<> dis(-1.0f, 1.0f);
        std::vector<glm::i8vec3> noiseTexData(16);
        for (auto& vec : noiseTexData) {
            vec.x = static_cast<int8_t>(dis(rng) * 128.0f);
            vec.y = static_cast<int8_t>(dis(rng) * 128.0f);
            vec.z = 0.0f;
        }
        ssaoNoise->SetData({ reinterpret_cast<std::byte*>(noiseTexData.data()), reinterpret_cast<std::byte*>(noiseTexData.data() + noiseTexData.size()) });

        std::vector<std::pair<std::string_view, std::shared_ptr<Gal::Texture>>> ssaoTextures = {
            {"normal", normal},
            {"light", light},
            {"depthStencil", depthStencil},
            {"ssaoNoise", ssaoNoise},
        };

        std::vector<std::pair<std::string_view, Gal::Type>> ssaoParameters = {
            {"ssaoSamples", Gal::Type::Int32},
        };

        ssaoPass = std::make_unique<PostProcess>(LoadPixelShader("/altcraft/shaders/frag/ssao"),
            ssaoTextures,
            ssaoParameters,
            ssaoW,
            ssaoH,
            Gal::Format::R8,
            Gal::Filtering::Bilinear);

        ssaoPass->SetShaderParameter("ssaoSamples", ssaoSamples);

        std::vector<std::pair<std::string_view, std::shared_ptr<Gal::Texture>>> ssaoBlurTextures = {
            {"blurInput", ssaoPass->GetResultTexture()},
        };

        std::vector<std::pair<std::string_view, Gal::Type>> ssaoBlurParameters = {
            {"blurScale", Gal::Type::Int32},
        };

        ssaoBlurPass = std::make_unique<PostProcess>(LoadPixelShader("/altcraft/shaders/frag/blur"),
            ssaoBlurTextures,
            ssaoBlurParameters,
            ssaoW,
            ssaoH,
            Gal::Format::R8,
            Gal::Filtering::Bilinear);

        ssaoBlurPass->SetShaderParameter("blurScale", 2);
    }

    std::vector<std::pair<std::string_view, Gal::Type>> lightingParameters = {
        {"renderBuff", Gal::Type::Int32},
        {"applySsao", Gal::Type::Int32},
    };

    std::vector<std::pair<std::string_view, std::shared_ptr<Gal::Texture>>> lightingTextures = {
        {"depthStencil", depthStencil},
        {"color", color},
        {"normal", normal},
        {"light", light},
    };

    if (ssaoSamples > 0)
        lightingTextures.emplace_back("ssao", ssaoBlurPass->GetResultTexture());

    lightingPass = std::make_unique<PostProcess>(LoadPixelShader("/altcraft/shaders/frag/light"),
        lightingTextures,
        lightingParameters,
        lightW,
        lightH,
        Gal::Format::R8G8B8,
        Gal::Filtering::Bilinear);

    lightingPass->SetShaderParameter("applySsao", ssaoSamples);
}

