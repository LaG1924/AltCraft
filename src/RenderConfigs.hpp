#pragma once

#include "Gal.hpp"

class Gbuffer {
    std::shared_ptr<Gal::Framebuffer> lightFramebuffer;
    std::shared_ptr<Gal::Buffer> lightBuffer;
    std::shared_ptr<Gal::Pipeline> lightPipeline;
    std::shared_ptr<Gal::PipelineInstance> lightPipelineInstance;
    std::shared_ptr<Gal::Texture> color; //RGB - color
    std::shared_ptr<Gal::Texture> normal; //RGB - normal
    std::shared_ptr<Gal::Texture> addColor; //RGB - addColor
    std::shared_ptr<Gal::Texture> light; //R - faceLight, G - skyLight, B - unused
    std::shared_ptr<Gal::Texture> depthStencil;
    std::shared_ptr<Gal::Framebuffer> geomFramebuffer;

    std::shared_ptr<Gal::Texture> finalColor;

public:
    Gbuffer(size_t geomW, size_t geomH, size_t lightW, size_t lightH);

    std::shared_ptr<Gal::Framebuffer> GetGeometryTarget() {
        return geomFramebuffer;
    }

    std::shared_ptr<Gal::Texture> GetFinalTexture() {
        return finalColor;
    }

    void Render() {
        lightPipeline->Activate();
        lightPipelineInstance->Activate();
        lightPipelineInstance->Render(0, 6);
    }

    void Clear() {
        geomFramebuffer->Clear();
        lightFramebuffer->Clear();
    }

    void SetDayTime(float dayTime) {
        lightPipeline->SetShaderParameter("dayTime", dayTime);
    }

    int GetMaxRenderBuffers() {
        return 5;
    }

    void SetRenderBuff(int renderBuff) {
        lightPipeline->SetShaderParameter("renderBuff", renderBuff);
    }
};
