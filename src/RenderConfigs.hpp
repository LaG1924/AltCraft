#pragma once

#include "Gal.hpp"

struct GlobalShaderParameters {
    glm::mat4 projView;
};

std::shared_ptr<Gal::Shader> LoadVertexShader(std::string_view assetPath);

std::shared_ptr<Gal::Shader> LoadPixelShader(std::string_view assetPath);

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
    std::unique_ptr<PostProcess> lightingPass;
    std::shared_ptr<Gal::Texture> color; //RGB - color
    std::shared_ptr<Gal::Texture> normal; //RGB - normal
    std::shared_ptr<Gal::Texture> addColor; //RGB - addColor
    std::shared_ptr<Gal::Texture> light; //R - faceLight, G - skyLight, B - unused
    std::shared_ptr<Gal::Texture> depthStencil;
    std::shared_ptr<Gal::Framebuffer> geomFramebuffer;

public:
    Gbuffer(size_t geomW, size_t geomH, size_t lightW, size_t lightH);

    std::shared_ptr<Gal::Framebuffer> GetGeometryTarget() {
        return geomFramebuffer;
    }

    std::shared_ptr<Gal::Texture> GetFinalTexture() {
        return lightingPass->GetResultTexture();
    }

    void Render() {
        lightingPass->Render();
    }

    void Clear() {
        geomFramebuffer->Clear();
        lightingPass->Clear();
    }

    void SetDayTime(float dayTime) {
        lightingPass->SetShaderParameter("dayTime", dayTime);
    }

    int GetMaxRenderBuffers() {
        return 5;
    }

    void SetRenderBuff(int renderBuff) {
        lightingPass->SetShaderParameter("renderBuff", renderBuff);
    }
};
