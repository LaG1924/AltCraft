#pragma once

#include "Gal.hpp"

struct GlobalShaderParameters {
    glm::mat4 projView;
    glm::mat4 proj;
    glm::mat4 invProj;
    glm::mat4 view;
    glm::uvec2 viewportSize;
    glm::uint32 paddingFA = 0xFAAFFAFA;
    glm::uint32 paddingFB = 0xFBFBFBFB;
    glm::vec4 ssaoKernels[64];
    glm::float32 globalTime;
    glm::float32 dayTime;
    glm::float32 gamma;
    glm::uint32 paddingF0 = 0xF0F0F0F0;
    glm::uint32 paddingF1 = 0xF1F1F1F1;
    glm::uint32 paddingF2 = 0xF2F2F2F2;
};

std::shared_ptr<Gal::Shader> LoadVertexShader(std::string_view assetPath);

std::shared_ptr<Gal::Shader> LoadPixelShader(std::string_view assetPath);

class TextureFbCopy {
    std::shared_ptr<Gal::Framebuffer> framebuffer;
    std::shared_ptr<Gal::Buffer> fbBuffer;
    std::shared_ptr<Gal::Pipeline> pipeline;
    std::shared_ptr<Gal::PipelineInstance> pipelineInstance;
public:

    TextureFbCopy(
        std::shared_ptr<Gal::Texture> inputTexture,
        std::shared_ptr<Gal::Texture> outputTexture,
        std::shared_ptr<Gal::Shader> copyShader = nullptr);

    TextureFbCopy(
        std::shared_ptr<Gal::Texture> inputTexture,
        std::shared_ptr<Gal::Framebuffer> outputFb,
        std::shared_ptr<Gal::Shader> copyShader = nullptr);

    void Clear() {
        framebuffer->Clear();
    }

    void Copy() {
        pipeline->Activate();
        pipelineInstance->Activate();
        pipelineInstance->Render(0, 6);
    }
};

class PostProcess {
    std::shared_ptr<Gal::Framebuffer> framebuffer;
    std::shared_ptr<Gal::Buffer> fbBuffer;
    std::shared_ptr<Gal::Pipeline> pipeline;
    std::shared_ptr<Gal::PipelineInstance> pipelineInstance;
    std::shared_ptr<Gal::Texture> result;
public:

    PostProcess(
        std::shared_ptr<Gal::Shader> pixelShader,
        std::vector<std::pair<std::string_view, std::shared_ptr<Gal::Texture>>> inputTextures,
        std::vector<std::pair<std::string_view, Gal::Type>> inputParameters,
        size_t width,
        size_t height,
        Gal::Format format,
        Gal::Filtering filtering);

    void Clear() {
        framebuffer->Clear();
    }

    void Render() {
        pipeline->Activate();
        pipelineInstance->Activate();
        pipelineInstance->Render(0, 6);
    }

    template<typename T>
    void SetShaderParameter(std::string_view name, T value) {
        pipeline->SetShaderParameter(name, value);
    }

    std::shared_ptr<Gal::Texture> GetResultTexture() {
        return result;
    }
};

class Gbuffer {
    std::shared_ptr<Gal::Texture> ssaoNoise;
    std::unique_ptr<PostProcess> ssaoPass;
    std::unique_ptr<PostProcess> ssaoBlurPass;
    std::unique_ptr<PostProcess> lightingPass;
    std::shared_ptr<Gal::Texture> depthStencil;
    std::shared_ptr<Gal::Texture> color; //RGB - color
    std::shared_ptr<Gal::Texture> normal; //RGB - normal
    std::shared_ptr<Gal::Texture> light; //R - faceLight, G - skyLight, B - ssaoDepthMask
    std::shared_ptr<Gal::Framebuffer> geomFramebuffer;

public:
    Gbuffer(size_t geomW, size_t geomH, size_t lightW, size_t lightH, int ssaoSamples, size_t ssaoW, size_t ssaoH);

    std::shared_ptr<Gal::Framebuffer> GetGeometryTarget() {
        return geomFramebuffer;
    }

    std::shared_ptr<Gal::Texture> GetFinalTexture() {
        return lightingPass->GetResultTexture();
    }

    void Render() {
        if (ssaoPass) {
            ssaoPass->Render();
            ssaoBlurPass->Render();
        }
        lightingPass->Render();
    }

    void Clear() {
        geomFramebuffer->Clear();
        if (ssaoPass) {
            ssaoPass->Clear();
            ssaoBlurPass->Clear();
        }
        lightingPass->Clear();
    }

    int GetMaxRenderBuffers() {
        return 7;
    }

    void SetRenderBuff(int renderBuff) {
        lightingPass->SetShaderParameter("renderBuff", renderBuff);
    }
};
