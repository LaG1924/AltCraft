#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <string_view>

#include <glm/glm.hpp>

namespace Gal {

    struct Impl;
    struct Buffer;
    struct BufferBinding;
    struct TextureConfig;
    struct Texture;
    struct PipelineConfig;
    struct Pipeline;
    struct PipelineInstance;
    struct FramebufferConfig;
    struct Framebuffer;
    struct ShaderParametersBuffer;
    struct Shader;


    enum class Type {
        Float,
        Double,
        Uint8,
        Uint16,
        Uint32,
        Int8,
        Int16,
        Int32,
        Vec2,
        Vec2u8,
        Vec2u16,
        Vec2u32,
        Vec2i8,
        Vec2i16,
        Vec2i32,
        Vec3,
        Vec3u8,
        Vec3u16,
        Vec3u32,
        Vec3i8,
        Vec3i16,
        Vec3i32,
        Vec4,
        Vec4u8,
        Vec4u16,
        Vec4u32,
        Vec4i8,
        Vec4i16,
        Vec4i32,
        Mat2,
        Mat3,
        Mat4,
    };

    enum class Format {
        D24S8,
        R8,
        R8G8,
        R8G8B8,
        R8G8B8SN,
        R8G8B8A8,
        R32G32B32A32F,
    };

    enum class Filtering {
        Nearest,
        Bilinear,
        Trilinear,
        Anisotropy,
    };

    enum class Wrapping {
        Repeat,
        Mirror,
        Clamp,
    };

    enum class Primitive {
        Line,
        Triangle,
        TriangleStrip,
        TriangleFan,
    };

    enum class Blending {
        Opaque,
        Additive,
    };

    struct VertexAttribute {
        std::string name;
        Type type = Type::Float;
        size_t count = 1;
        size_t instances = 0;
    };

    Impl* GetImplementation();

    struct Impl {

        virtual void Init() = 0;

        virtual void DeInit() = 0;

        virtual void Cleanup() = 0;

        virtual void SetScissor(size_t x=0, size_t y=0, size_t width=0, size_t height=0) = 0;

        virtual void SetScissor(bool enabled) = 0;

        virtual void SetWireframe(bool enabled) = 0;


        virtual std::shared_ptr<Buffer> CreateBuffer() = 0;


        virtual std::shared_ptr<TextureConfig> CreateTexture2DConfig(size_t width, size_t height, Format format) = 0;

        virtual std::shared_ptr<TextureConfig> CreateTexture3DConfig(size_t width, size_t height, size_t depth, bool interpolateLayers, Format format) = 0;

        virtual std::shared_ptr<Texture> BuildTexture(std::shared_ptr<TextureConfig> config) = 0;


        virtual std::shared_ptr<PipelineConfig> CreatePipelineConfig() = 0;

        virtual std::shared_ptr<Pipeline> BuildPipeline(std::shared_ptr<PipelineConfig> config) = 0;


        virtual std::shared_ptr<FramebufferConfig> CreateFramebufferConfig() = 0;

        virtual std::shared_ptr<Framebuffer> BuildFramebuffer(std::shared_ptr<FramebufferConfig> config) = 0;

        virtual std::shared_ptr<Framebuffer> GetDefaultFramebuffer() = 0;


        virtual std::shared_ptr<ShaderParametersBuffer> GetGlobalShaderParameters() = 0;

        virtual std::shared_ptr<Shader> LoadVertexShader(std::string_view code) = 0;

        virtual std::shared_ptr<Shader> LoadPixelShader(std::string_view code) = 0;

    };

    struct Buffer {
        virtual ~Buffer() = default;

        virtual void SetData(std::vector<std::byte>&& data) = 0;
    };

    struct BufferBinding {
        virtual ~BufferBinding() = default;
    };

    struct TextureConfig {
        virtual ~TextureConfig() = default;

        virtual void SetMinFilter(Filtering filter) = 0;

        virtual void SetMaxFilter(Filtering filter) = 0;

        virtual void SetWrapping(Wrapping wrapping) = 0;

        virtual void SetLinear(bool isLinear) = 0;

    };

    struct Texture {
        virtual ~Texture() = default;

        virtual std::tuple<size_t, size_t, size_t> GetSize() = 0;

        virtual void SetData(std::vector<std::byte>&& data, size_t mipLevel = 0) = 0;

        virtual void SetSubData(size_t x, size_t y, size_t z, size_t width, size_t height, size_t depth, std::vector<std::byte> &&data, size_t mipLevel = 0) = 0;
    };

    struct PipelineConfig {
        virtual ~PipelineConfig() = default;

        virtual void SetVertexShader(std::shared_ptr<Shader> shader) = 0;

        virtual void SetPixelShader(std::shared_ptr<Shader> shader) = 0;

        virtual void AddShaderParameter(std::string_view name, Type type) = 0;

        virtual void AddStaticTexture(std::string_view name, std::shared_ptr<Texture> texture) = 0;

        virtual void SetTarget(std::shared_ptr<Framebuffer> target) = 0;

        virtual void SetPrimitive(Primitive primitive) = 0;

        virtual void SetBlending(Blending blendingMode) = 0;

        virtual std::shared_ptr<BufferBinding> BindVertexBuffer(std::vector<VertexAttribute> &&bufferLayout) = 0;

        virtual std::shared_ptr<BufferBinding> BindIndexBuffer() = 0;
    };

    struct Pipeline {
        virtual ~Pipeline() = default;

        virtual void Activate() = 0;

        virtual std::shared_ptr<PipelineInstance> CreateInstance(std::vector<std::pair<std::shared_ptr<BufferBinding>, std::shared_ptr<Buffer>>> &&buffers) = 0;

        virtual void SetDynamicTexture(std::string_view name, std::shared_ptr<Texture> texture) = 0;

        virtual void SetShaderParameter(std::string_view name, float value) = 0;

        virtual void SetShaderParameter(std::string_view name, double value) = 0;

        virtual void SetShaderParameter(std::string_view name, int8_t value) = 0;

        virtual void SetShaderParameter(std::string_view name, int16_t value) = 0;

        virtual void SetShaderParameter(std::string_view name, int32_t value) = 0;

        virtual void SetShaderParameter(std::string_view name, uint8_t value) = 0;

        virtual void SetShaderParameter(std::string_view name, uint16_t value) = 0;

        virtual void SetShaderParameter(std::string_view name, uint32_t value) = 0;

        virtual void SetShaderParameter(std::string_view name, glm::vec2 value) = 0;

        virtual void SetShaderParameter(std::string_view name, glm::uvec2 value) = 0;

        virtual void SetShaderParameter(std::string_view name, glm::vec3 value) = 0;

        virtual void SetShaderParameter(std::string_view name, glm::vec4 value) = 0;

        virtual void SetShaderParameter(std::string_view name, glm::mat4 value) = 0;
    };

    struct PipelineInstance {
        virtual ~PipelineInstance() = default;

        virtual void Activate() = 0;

        virtual void Render(size_t offset = 0, size_t count = -1) = 0;

        virtual void SetInstancesCount(size_t count) = 0;
    };

    struct Framebuffer {
        virtual ~Framebuffer() = default;

        virtual void Clear() = 0;

        virtual void SetViewport(size_t x, size_t y, size_t w, size_t h) = 0;
    };

    struct FramebufferConfig {
        virtual ~FramebufferConfig() = default;

        virtual void SetDepthStencil(std::shared_ptr<Texture> texture) = 0;

        virtual void SetTexture(size_t location, std::shared_ptr<Texture> texture) = 0;
    };

    struct ShaderParametersBuffer {
        virtual ~ShaderParametersBuffer() = default;

        template<typename T>
        T* Get() {
            return reinterpret_cast<T*>(GetDataPtr());
        }

        template<typename T>
        void Resize() {
            Resize(sizeof(T));
            *Get<T>() = T{};
        }

        virtual std::byte* GetDataPtr() = 0;

        virtual void Resize(size_t newSize) = 0;
    };

    struct Shader {
        virtual ~Shader() = default;
    };
}
