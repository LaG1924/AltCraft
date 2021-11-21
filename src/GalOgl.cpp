#include "Gal.hpp"

#include <easylogging++.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <optick.h>

#include "Utility.hpp"

enum class GlResourceType {
    Vbo,
    Vao,
    Texture,
    Fbo,
    Program,
    None,
};

class GlResource {
    GlResourceType type = GlResourceType::None;
    GLuint res = 0;
public:
    GlResource() = default;

    GlResource(GLuint resource, GlResourceType resType) noexcept : res(resource), type(resType) {}

    GlResource(const GlResource&) = delete;

    GlResource(GlResource&& rhs) noexcept {
        std::swap(this->res, rhs.res);
        std::swap(this->type, rhs.type);
    }

    GlResource& operator=(const GlResource&) = delete;

    GlResource& operator=(GlResource&& rhs) noexcept {
        std::swap(this->res, rhs.res);
        std::swap(this->type, rhs.type);
        return *this;
    }

    ~GlResource() {
        switch (type) {
        case GlResourceType::Vbo:
            glDeleteBuffers(1, &res);
            break;
        case GlResourceType::Vao:
            glDeleteVertexArrays(1, &res);
            break;
        case GlResourceType::Texture:
            glDeleteTextures(1, &res);
            break;
        case GlResourceType::Fbo:
            glDeleteFramebuffers(1, &res);
            break;
        case GlResourceType::Program:
            glDeleteProgram(res);
            break;
        case GlResourceType::None:
        default:
            break;
        }
    }

    operator GLuint() const noexcept {
        return res;
    }

    GLuint Get() const noexcept {
        return res;
    }
};


using namespace Gal;

class ImplOgl;
class ShaderOgl;
class FramebufferOgl;

class OglState {
    GLuint activeFbo = 0;
    GLuint activeVao = 0;
    GLuint activeVbo = 0;
    GLuint activeEbo = 0;
    GLuint activeProgram = 0;
    GLuint activeTexture[16] = { 0 };
    GLuint activeTextureUnit = 0;
    GLint vpX = 0, vpY = 0;
    GLsizei vpW = 0, vpH = 0;

public:

    void BindFbo(GLuint fbo) {
        if (fbo != activeFbo) {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            activeFbo = fbo;
        }
        glCheckError();
    }

    void BindVao(GLuint vao) {
        if (vao != activeVao) {
            glBindVertexArray(vao);
            activeVao = vao;
        }
        glCheckError();
    }

    void BindVbo(GLuint vbo) {
        if (vbo != activeVbo) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            activeVbo = vbo;
        }
        glCheckError();
    }

    void BindEbo(GLuint ebo) {
        if (ebo != activeEbo) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            activeEbo = ebo;
        }
        glCheckError();
    }

    void SetTextureUnit(size_t textureUnit) {
        if (textureUnit != activeTextureUnit) {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            activeTextureUnit = textureUnit;
        }
        glCheckError();
    }

    void BindTexture(GLenum type, GLuint texture, size_t textureUnit = 17) {
        if (textureUnit >= 16)
            textureUnit = activeTextureUnit;
        if (activeTexture[textureUnit] != texture) {
            SetTextureUnit(textureUnit);
            glBindTexture(type, texture);
            activeTexture[textureUnit] = texture;
        }
        glCheckError();
    }

    void UseProgram(GLuint program) {
        if (program != activeProgram) {
            glUseProgram(program);
            activeProgram = program;
        }
        glCheckError();
    }

    void SetViewport(GLint x, GLint y, GLsizei w, GLsizei h) {
        if (x != vpX || y != vpY || w != vpW || h != vpH) {
            glViewport(x, y, w, h);
            vpX = x;
            vpY = y;
            vpW = w;
            vpH = h;
        }
        glCheckError();
    }

} oglState;

std::unique_ptr<ImplOgl> impl;
std::shared_ptr<FramebufferOgl> fbDefault;

size_t GalTypeGetComponents(Gal::Type type) {
    switch (type) {
    case Type::Float:
    case Type::Double:
    case Type::Uint8:
    case Type::Uint16:
    case Type::Uint32:
    case Type::Int8:
    case Type::Int16:
    case Type::Int32:
        return 1;
    case Type::Vec2:
    case Type::Vec2u8:
    case Type::Vec2u16:
    case Type::Vec2u32:
    case Type::Vec2i8:
    case Type::Vec2i16:
    case Type::Vec2i32:
        return 2;
    case Type::Vec3:
    case Type::Vec3u8:
    case Type::Vec3u16:
    case Type::Vec3u32:
    case Type::Vec3i8:
    case Type::Vec3i16:
    case Type::Vec3i32:
        return 3;
    case Type::Vec4:
    case Type::Vec4u8:
    case Type::Vec4u16:
    case Type::Vec4u32:
    case Type::Vec4i8:
    case Type::Vec4i16:
    case Type::Vec4i32:
    case Type::Mat2:
        return 4;
    case Type::Mat3:
        return 9;
    case Type::Mat4:
        return 16;
    default:
        return 0;
    }
    return 0;
}

size_t GalTypeGetComponentSize(Gal::Type type) {
    switch (type) {	
    case Type::Uint8:
    case Type::Int8:
    case Type::Vec2u8:
    case Type::Vec2i8:
    case Type::Vec3u8:
    case Type::Vec3i8:
    case Type::Vec4u8:
    case Type::Vec4i8:
        return 1;
    case Type::Uint16:
    case Type::Int16:
    case Type::Vec2u16:
    case Type::Vec2i16:
    case Type::Vec3u16:
    case Type::Vec3i16:
    case Type::Vec4u16:
    case Type::Vec4i16:
        return 2;
    case Type::Float:
    case Type::Uint32:
    case Type::Int32:
    case Type::Vec2:
    case Type::Vec2u32:
    case Type::Vec2i32:
    case Type::Vec3:
    case Type::Vec3u32:
    case Type::Vec3i32:
    case Type::Vec4:
    case Type::Vec4u32:
    case Type::Vec4i32:
    case Type::Mat2:
    case Type::Mat3:
    case Type::Mat4:
        return 4;
    case Type::Double:
        return 8;
    default:
        return 0;
    }
}

size_t GalTypeGetSize(Gal::Type type) {
    return GalTypeGetComponents(type) * GalTypeGetComponentSize(type);
}

GLenum GalTypeGetComponentGlType(Gal::Type type) {
    switch (type) {
    case Type::Float:
    case Type::Vec2:
    case Type::Vec3:
    case Type::Vec4:
    case Type::Mat2:
    case Type::Mat3:
    case Type::Mat4:
        return GL_FLOAT;
    case Type::Double:
        return GL_DOUBLE;
    case Type::Uint8:
    case Type::Vec2u8:
    case Type::Vec3u8:
    case Type::Vec4u8:
        return GL_UNSIGNED_BYTE;
    case Type::Uint16:
    case Type::Vec2u16:
    case Type::Vec3u16:
    case Type::Vec4u16:
        return GL_UNSIGNED_SHORT;
    case Type::Uint32:
    case Type::Vec2u32:
    case Type::Vec3u32:
    case Type::Vec4u32:
        return GL_UNSIGNED_INT;
    case Type::Int8:
    case Type::Vec2i8:
    case Type::Vec3i8:
    case Type::Vec4i8:
        return GL_BYTE;
    case Type::Int16:
    case Type::Vec2i16:
    case Type::Vec3i16:
    case Type::Vec4i16:
        return GL_SHORT;
    case Type::Int32:
    case Type::Vec2i32:
    case Type::Vec3i32:
    case Type::Vec4i32:
        return GL_INT;
    default:
        return 0;
    }
    return 0;
}

size_t GalFormatGetSize(Format format) {
    switch (format) {
    case Format::R8G8B8:
        return 3;
    case Format::R8G8B8A8:
        return 4;
    default:
        return 0;
    }
    return 0;
}

GLenum GalFormatGetGlInternalFormat(Format format) {
    switch (format) {
    case Format::D24S8:
        return GL_DEPTH24_STENCIL8;
    case Format::R8G8B8:
        return GL_RGB8;
    case Format::R8G8B8A8:
        return GL_RGBA8;
    default:
        return 0;
    }
    return 0;
}

GLenum GalFormatGetGlFormat(Format format) {
    switch (format) {
    case Format::D24S8:
        return GL_DEPTH_STENCIL;
    case Format::R8G8B8:
        return GL_RGB;
    case Format::R8G8B8A8:
        return GL_RGBA;
    default:
        return 0;
    }
    return 0;
}

GLenum GalFormatGetGlType(Format format) {
    switch (format) {
    case Format::D24S8:
        return GL_UNSIGNED_INT_24_8;
    case Format::R8G8B8:
        return GL_UNSIGNED_BYTE;
    case Format::R8G8B8A8:
        return GL_UNSIGNED_BYTE;
    default:
        return 0;
    }
    return 0;
}

GLenum GalFilteringGetGlType(Filtering filtering) {
    switch (filtering) {
    case Filtering::Nearest:
        return GL_NEAREST;
    case Filtering::Bilinear:
        return GL_LINEAR;
    case Filtering::Trilinear:
        return GL_LINEAR_MIPMAP_LINEAR;
    case Filtering::Anisotropy:
        return GL_LINEAR;
    default:
        return 0;
    }
    return 0;
}

GLenum GalWrappingGetGlType(Wrapping wrapping) {
    switch (wrapping) {
    case Wrapping::Repeat:
        return GL_REPEAT;
    case Wrapping::Clamp:
        return GL_CLAMP_TO_EDGE;
    case Wrapping::Mirror:
        return GL_MIRRORED_REPEAT;
    default:
        return 0;
    }
    return 0;
}

GLenum glCheckError_(const char* file, int line) {
    OPTICK_EVENT();
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        LOG(ERROR) << "OpenGL error: " << error << " at " << file << ":" << line;
    }
    return errorCode;
}

#ifndef NDEBUG
#define glCheckError() glCheckError_(__FILE__, __LINE__)
#else
#define glCheckError()
#endif // !NDEBUG


void GLAPIENTRY glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    el::Level level = el::Level::Error;
    std::string sourceText;
    std::string typeText;
    std::string severityText;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        severityText = "HIGH";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severityText = "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        severityText = "LOW";
        level = el::Level::Warning;
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        severityText = "NOTIFY";
        level = el::Level::Info;
        break;
    }

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        sourceText = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        sourceText = "Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        sourceText = "Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        sourceText = "Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        sourceText = "Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        sourceText = "Other";
        break;
    default:
        sourceText = std::to_string(source);
        break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        typeText = "Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        typeText = "Deprecated Behaviour"; 
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        typeText = "Undefined Behaviour"; 
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        typeText = "Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        typeText = "Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        typeText = "Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        typeText = "Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        typeText = "Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        typeText = "Other";
        break;
    default:
        typeText = std::to_string(type);
        break;
    }

    std::string log = "OpenGL debug (" + std::to_string(id) + ") [" + severityText + "][" + sourceText + "][" + typeText + "]: \n" + message;

    switch (level) {
    case el::Level::Error:
        LOG(ERROR) << log;
        break;
    case el::Level::Warning:
        LOG(WARNING) << log;
        break;
    case el::Level::Info:
        LOG(INFO) << log;
        break;
    default:
        LOG(ERROR) << log;
        break;
    }
}


struct ShaderOgl : public Shader {

    bool isVertex = true;
    std::string code;

};

struct BufferBindingOgl : public BufferBinding {

    BufferBindingOgl(size_t id) : bufferId(id) {}

    const size_t bufferId;

    static constexpr size_t indexValue = (std::numeric_limits<size_t>::max)(); //parenthess for windows' max macro

};

struct BufferOgl : public Buffer {

    GlResource vbo;

    virtual void SetData(std::vector<std::byte>&& data) override {
        oglState.BindVbo(vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size(), data.data(), GL_STATIC_DRAW);
        oglState.BindVbo(0);
        glCheckError();
    }

};

struct TextureConfigOgl : public TextureConfig {

    Format format;
    size_t width = 1, height = 1, depth = 1;
    bool interpolateLayers = false;
    GLenum type;

    Filtering min = Filtering::Nearest, max = Filtering::Nearest;
    Wrapping wrap = Wrapping::Clamp;

    virtual void SetMinFilter(Filtering filter) override {
        min = filter;
    }

    virtual void SetMaxFilter(Filtering filter) override {
        max = filter;
    }

    virtual void SetWrapping(Wrapping wrapping) override {
        wrap = wrapping;
    }

};

struct TextureOgl : public Texture {

    GLenum type;
    GlResource texture;
    Format format;
    size_t width, height, depth;

    virtual void SetData(std::vector<std::byte>&& data, size_t mipLevel = 0) override {
        size_t expectedSize = width * height * depth * GalFormatGetSize(format);
        if (data.size() != expectedSize && !data.empty())
            throw std::logic_error("Size of data is not valid for this texture");

        oglState.BindTexture(type, texture);

        switch (type) {
        case GL_TEXTURE_1D:
        case GL_PROXY_TEXTURE_1D:
            break;
        case GL_TEXTURE_2D:
        case GL_PROXY_TEXTURE_2D:
        case GL_TEXTURE_1D_ARRAY:
        case GL_PROXY_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_RECTANGLE:
        case GL_PROXY_TEXTURE_RECTANGLE:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        case GL_PROXY_TEXTURE_CUBE_MAP:
            glTexImage2D(type, mipLevel, GalFormatGetGlInternalFormat(format), width, height, 0, GalFormatGetGlFormat(format), GalFormatGetGlType(format), data.empty() ? nullptr : data.data());
            break;
        case GL_TEXTURE_3D:
        case GL_PROXY_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY:
        case GL_PROXY_TEXTURE_2D_ARRAY:
            glTexImage3D(type, mipLevel, GalFormatGetGlInternalFormat(format), width, height, depth, 0, GalFormatGetGlFormat(format), GalFormatGetGlType(format), data.empty() ? nullptr : data.data());
            break;
        default:
            throw std::runtime_error("Unknown texture type");
        }

        glCheckError();
        oglState.BindTexture(type, 0);
    }

    virtual void SetSubData(size_t x, size_t y, size_t z, size_t width, size_t height, size_t depth, std::vector<std::byte>&& data, size_t mipLevel = 0) override {
        size_t expectedSize = width * height * depth * GalFormatGetSize(format);
        if (data.size() != expectedSize)
            throw std::logic_error("Size of data is not valid for this texture");

        oglState.BindTexture(type, texture);

        switch (type) {
        case GL_TEXTURE_1D:
        case GL_PROXY_TEXTURE_1D:
            break;
        case GL_TEXTURE_2D:
        case GL_PROXY_TEXTURE_2D:
        case GL_TEXTURE_1D_ARRAY:
        case GL_PROXY_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_RECTANGLE:
        case GL_PROXY_TEXTURE_RECTANGLE:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        case GL_PROXY_TEXTURE_CUBE_MAP:
            glTexSubImage2D(type, mipLevel, x, y, width, height, GalFormatGetGlFormat(format), GalFormatGetGlType(format), data.data());
            break;
        case GL_TEXTURE_3D:
        case GL_PROXY_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY:
        case GL_PROXY_TEXTURE_2D_ARRAY:
            glTexSubImage3D(type, mipLevel, x, y, z, width, height, depth, GalFormatGetGlFormat(format), GalFormatGetGlType(format), data.data());
            break;
        default:
            throw std::runtime_error("Unknown texture type");
        }

        glCheckError();
        oglState.BindTexture(type, 0);
    }

};

struct FramebufferOgl : public Framebuffer {

    size_t vpX = 0, vpY = 0, vpW = 1, vpH = 1;
    std::shared_ptr<TextureOgl> depthStencil;
    std::vector<std::shared_ptr<TextureOgl>> colors;
    std::vector<GLenum> attachments;

    GlResource fbo;

    virtual void Clear() override {
        oglState.BindFbo(fbo ? fbo : 0);
        GLbitfield clearBits = 0;
        clearBits |= GL_COLOR_BUFFER_BIT;
        clearBits |= GL_DEPTH_BUFFER_BIT;
        if (depthStencil)
            clearBits |= GL_STENCIL_BUFFER_BIT;
        glClear(clearBits);
        glCheckError();
    }

    virtual void SetViewport(size_t x, size_t y, size_t w, size_t h) override {
        vpX = x;
        vpY = y;
        vpW = w;
        vpH = h;
    }

};

struct FramebufferConfigOgl : public FramebufferConfig {

    std::shared_ptr<TextureOgl> depthStencil;
    std::map<size_t, std::shared_ptr<TextureOgl>> colors;

    virtual void SetDepthStencil(std::shared_ptr<Texture> texture) override {
        auto tex = std::static_pointer_cast<TextureOgl, Texture>(texture);
        depthStencil = tex;
    }

    virtual void SetTexture(size_t location, std::shared_ptr<Texture> texture) override {
        auto tex = std::static_pointer_cast<TextureOgl, Texture>(texture);
        colors.emplace(location, tex);
    }
};

struct PipelineConfigOgl : public PipelineConfig {

    std::shared_ptr<ShaderOgl> vertexShader, pixelShader;
    std::map<std::string, std::shared_ptr<TextureOgl>> textures;
    std::map<std::string, Type> shaderParameters;
    std::shared_ptr<FramebufferOgl> targetFb;
    std::vector<std::vector<VertexAttribute>> vertexBuffers;
    Primitive vertexPrimitive = Primitive::Triangle;

    virtual void SetVertexShader(std::shared_ptr<Shader> shader) override {
        vertexShader = std::static_pointer_cast<ShaderOgl,Shader>(shader);
    }

    virtual void SetPixelShader(std::shared_ptr<Shader> shader) override {
        pixelShader = std::static_pointer_cast<ShaderOgl, Shader>(shader);
    }

    virtual void AddShaderParameter(std::string_view name, Type type) override {
        shaderParameters.emplace(std::string(name), type);
    }

    virtual void AddStaticTexture(std::string_view name, std::shared_ptr<Texture> texture) override {
        auto tex = std::static_pointer_cast<TextureOgl, Texture>(texture);
        textures.emplace(std::string(name), tex);
    }

    virtual void SetTarget(std::shared_ptr<Framebuffer> target) override {
        auto fb = std::static_pointer_cast<FramebufferOgl, Framebuffer>(target);
        targetFb = fb;
    }

    virtual void SetPrimitive(Primitive primitive) override {
        vertexPrimitive = primitive;
    }

    virtual std::shared_ptr<BufferBinding> BindVertexBuffer(std::vector<VertexAttribute> &&bufferLayout) override {
        auto binding = std::make_shared<BufferBindingOgl>(vertexBuffers.size());
        vertexBuffers.push_back(bufferLayout);
        return std::static_pointer_cast<BufferBinding, BufferBindingOgl>(binding);
    }

    virtual std::shared_ptr<BufferBinding> BindIndexBuffer() override {
        auto binding = std::make_shared<BufferBindingOgl>(BufferBindingOgl::indexValue);
        return std::static_pointer_cast<BufferBinding, BufferBindingOgl>(binding);
    }

};

struct PipelineInstanceOgl : public PipelineInstance {

    GlResource vao;
    bool useIndex = false;
    Primitive primitive;
    size_t instances = 0;

    virtual void Activate() override {
        oglState.BindVao(vao);
    }

    virtual void Render(size_t offset = 0, size_t count = -1) override {
        GLenum vertexMode;
        switch (primitive) {
        case Primitive::Line:
            vertexMode = GL_LINES;
            break;
        case Primitive::Triangle:
            vertexMode = GL_TRIANGLES;
            break;
        case Primitive::TriangleFan:
            vertexMode = GL_TRIANGLE_FAN;
            break;
        case Primitive::TriangleStrip:
            vertexMode = GL_TRIANGLE_STRIP;
            break;
        default:
            vertexMode = GL_TRIANGLES;
        }

        if (useIndex) {
            if (instances) {
                glDrawElementsInstanced(vertexMode, instances, GL_UNSIGNED_INT, nullptr, instances);
            }
            else {
                glDrawElements(vertexMode, count, GL_UNSIGNED_INT, nullptr);
            }
        }
        else {
            if (instances) {
                glDrawArraysInstanced(vertexMode, offset, instances, count);
            }
            else {
                glDrawArrays(vertexMode, offset, count);
            }
        }
    }

    virtual void SetInstancesCount(size_t count) override {
        instances = count;
    }

};

struct PipelineOgl : public Pipeline {

    std::map<std::string, size_t> shaderParameters;
    std::vector<std::shared_ptr<TextureOgl>> staticTextures;
    GlResource program;
    struct VertexBindingCommand {
        size_t bufferId;
        size_t location;
        GLenum type;
        size_t count;
        size_t stride;
        size_t offset;
        size_t instances;
    };
    std::vector<VertexBindingCommand> vertexBindCmds;
    Primitive primitive;
    std::shared_ptr<FramebufferOgl> target;
    
    virtual void Activate() override {
        oglState.UseProgram(program);
        oglState.BindFbo(target->fbo);
        oglState.SetViewport(target->vpX, target->vpY, target->vpW, target->vpH);
        if (target->fbo)
            glDrawBuffers(target->attachments.size(), target->attachments.data());

        for (size_t i = 0; i < staticTextures.size(); i++) {
            oglState.BindTexture(staticTextures[i]->type, staticTextures[i]->texture, i);
        }
    }

    virtual void SetDynamicTexture(std::string_view name, std::shared_ptr<Texture> texture) override {
        Activate();
        auto tex = std::static_pointer_cast<TextureOgl>(texture);
        oglState.BindTexture(tex->type, tex->texture, staticTextures.size());
        SetShaderParameter(name, static_cast<int>(staticTextures.size()));
        glCheckError();
    }

    virtual std::shared_ptr<PipelineInstance> CreateInstance(std::vector<std::pair<std::shared_ptr<BufferBinding>, std::shared_ptr<Buffer>>>&& buffers) override {
        auto instance = std::make_shared<PipelineInstanceOgl>();

        instance->primitive = primitive;

        size_t indexBuffer = BufferBindingOgl::indexValue;
        std::map<size_t, size_t> bufferBindingId;

        for (auto&& [binding, buffer] : buffers) {
            auto bind = std::static_pointer_cast<BufferBindingOgl, BufferBinding>(binding);
            auto buff = std::static_pointer_cast<BufferOgl, Buffer>(buffer);

            if (bind->bufferId == BufferBindingOgl::indexValue)
                indexBuffer = buff->vbo;
            else
                bufferBindingId.insert({ bind->bufferId,buff->vbo });
        }

        GLuint newVao;
        glGenVertexArrays(1, &newVao);
        instance->vao = GlResource(newVao, GlResourceType::Vao);
        oglState.BindVao(instance->vao);

        for (const auto& cmd : vertexBindCmds) {
            oglState.BindVbo(bufferBindingId.find(cmd.bufferId)->second);
            switch (cmd.type) {
            case GL_FLOAT:
            case GL_DOUBLE:
                glVertexAttribPointer(cmd.location, cmd.count, cmd.type, GL_FALSE, cmd.offset, reinterpret_cast<void*>(cmd.stride));
                break;
            case GL_UNSIGNED_BYTE:
            case GL_BYTE:
            case GL_UNSIGNED_SHORT:
            case GL_SHORT:
            case GL_UNSIGNED_INT:
            case GL_INT:
                glVertexAttribIPointer(cmd.location, cmd.count, cmd.type, cmd.offset, reinterpret_cast<void*>(cmd.stride));
                break;
            }
            
            glEnableVertexAttribArray(cmd.location);
            if (cmd.instances) {
                glVertexAttribDivisor(cmd.location, cmd.instances);
            }
        }

        if (indexBuffer != BufferBindingOgl::indexValue) {
            oglState.BindEbo(indexBuffer);
            instance->useIndex = true;
        }

        glCheckError();

        oglState.BindVao(0);
        oglState.BindVbo(0);
        oglState.BindEbo(0);

        return instance;
    }

    virtual void SetShaderParameter(std::string_view name, float value) override {
        Activate();
        glUniform1f(shaderParameters.at(std::string(name)), value);
    }

    virtual void SetShaderParameter(std::string_view name, double value) override {
        Activate();
        glUniform1d(shaderParameters.at(std::string(name)), value);
    }

    virtual void SetShaderParameter(std::string_view name, int8_t value) override {
        Activate();
        glUniform1i(shaderParameters.at(std::string(name)), value);
    }

    virtual void SetShaderParameter(std::string_view name, int16_t value) override {
        Activate();
        glUniform1i(shaderParameters.at(std::string(name)), value);
    }

    virtual void SetShaderParameter(std::string_view name, int32_t value) override {
        Activate();
        glUniform1i(shaderParameters.at(std::string(name)), value);
    }

    virtual void SetShaderParameter(std::string_view name, uint8_t value) override {
        Activate();
        glUniform1ui(shaderParameters.at(std::string(name)), value);
    }

    virtual void SetShaderParameter(std::string_view name, uint16_t value) override {
        Activate();
        glUniform1ui(shaderParameters.at(std::string(name)), value);
    }

    virtual void SetShaderParameter(std::string_view name, uint32_t value) override {
        Activate();
        glUniform1ui(shaderParameters.at(std::string(name)), value);
    }

    virtual void SetShaderParameter(std::string_view name, glm::vec2 value) override {
        Activate();
        glUniform2f(shaderParameters.at(std::string(name)), value.x, value.y);
    }

    virtual void SetShaderParameter(std::string_view name, glm::uvec2 value) override {
        Activate();
        glUniform2ui(shaderParameters.at(std::string(name)), value.x, value.y);
    }

    virtual void SetShaderParameter(std::string_view name, glm::vec3 value) override {
        Activate();
        glUniform3f(shaderParameters.at(std::string(name)), value.x, value.y, value.z);
    }

    virtual void SetShaderParameter(std::string_view name, glm::vec4 value) override {
        Activate();
        glUniform4f(shaderParameters.at(std::string(name)), value.x, value.y, value.z, value.w);
    }

    virtual void SetShaderParameter(std::string_view name, glm::mat4 value) override {
        Activate();
        glUniformMatrix4fv(shaderParameters.at(std::string(name)), 1, GL_FALSE, glm::value_ptr(value));
    }

};

struct ImplOgl : public Impl {

    virtual void Init() override {
        LOG(INFO) << "Initalizing Gal:OpenGL...";
        LOG(INFO) << "Initializing GLEW";
        glewExperimental = GL_TRUE;
        GLenum glewStatus = glewInit();
        if (glewStatus != GLEW_OK) {
            LOG(FATAL) << "Failed to initialize GLEW: " << glewGetErrorString(glewStatus);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glCheckError();
        if (glActiveTexture == nullptr) {
            throw std::runtime_error("GLEW initialization failed with unknown reason");
        }

        GLint flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
        glCheckError();
    }

    virtual void DeInit() override {
        LOG(INFO) << "Destroying Gal:OpenGL...";
        glCheckError();
    }

    virtual void Cleanup() override {

    }

    virtual void SetScissor(size_t x = 0, size_t y = 0, size_t width = 0, size_t height = 0) override {
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y, width, height);
        glCheckError();
    }

    virtual void SetScissor(bool enabled) override {
        if (enabled)
            glEnable(GL_SCISSOR_TEST);
        else
            glDisable(GL_SCISSOR_TEST);
        glCheckError();
    }

    virtual void SetWireframe(bool enabled) override {
        glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
    }


    virtual std::shared_ptr<Buffer> CreateBuffer() override {
        auto buff = std::make_shared<BufferOgl>();
        GLuint newVbo;
        glGenBuffers(1, &newVbo);
        buff->vbo = GlResource(newVbo, GlResourceType::Vbo);
        buff->SetData({});
        glCheckError();
        return std::static_pointer_cast<Buffer, BufferOgl>(buff);
    }


    virtual std::shared_ptr<TextureConfig> CreateTexture2DConfig(size_t width, size_t height, Format format) override {
        auto config = std::make_shared<TextureConfigOgl>();

        config->type = GL_TEXTURE_2D;
        config->width = width;
        config->height = height;
        config->depth = 1;
        config->format = format;

        return std::static_pointer_cast<TextureConfig, TextureConfigOgl>(config);
    }

    virtual std::shared_ptr<TextureConfig> CreateTexture3DConfig(size_t width, size_t height, size_t depth, bool interpolateLayers, Format format) override {
        auto config = std::make_shared<TextureConfigOgl>();

        config->type = interpolateLayers ? GL_TEXTURE_3D : GL_TEXTURE_2D_ARRAY;
        config->width = width;
        config->height = height;
        config->depth = depth;
        config->interpolateLayers = interpolateLayers;
        config->format = format;

        return std::static_pointer_cast<TextureConfig, TextureConfigOgl>(config);
    }

    virtual std::shared_ptr<Texture> BuildTexture(std::shared_ptr<TextureConfig> config) override {
        auto texConfig = std::static_pointer_cast<TextureConfigOgl, TextureConfig>(config);
        auto texture = std::make_shared<TextureOgl>();

        texture->type = texConfig->type;
        texture->format = texConfig->format;
        texture->width = texConfig->width;
        texture->height = texConfig->height;
        texture->depth = texConfig->depth;

        GLuint newTex;
        glGenTextures(1, &newTex);
        texture->texture = GlResource(newTex, GlResourceType::Texture);
        
        oglState.BindTexture(texture->type, texture->texture);

        glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER, GalFilteringGetGlType(texConfig->min));
        glTexParameteri(texture->type, GL_TEXTURE_MAG_FILTER, GalFilteringGetGlType(texConfig->max));
        glTexParameteri(texture->type, GL_TEXTURE_WRAP_S, GalWrappingGetGlType(texConfig->wrap));
        glTexParameteri(texture->type, GL_TEXTURE_WRAP_T, GalWrappingGetGlType(texConfig->wrap));

        oglState.BindTexture(texture->type, 0);
        texture->SetData(std::vector<std::byte>(texture->width * texture->height * texture->depth * GalFormatGetSize(texture->format)));
        glCheckError();

        return std::static_pointer_cast<Texture, TextureOgl>(texture);
    }


    virtual std::shared_ptr<PipelineConfig> CreatePipelineConfig() override {
        auto pipelineConfig = std::make_shared<PipelineConfigOgl>();
        return std::static_pointer_cast<PipelineConfig, PipelineConfigOgl>(pipelineConfig);
    }

    virtual std::shared_ptr<Pipeline> BuildPipeline(std::shared_ptr<PipelineConfig> pipelineConfig) override {
        auto pipeline = std::make_shared<PipelineOgl>();
        auto config = std::static_pointer_cast<PipelineConfigOgl, PipelineConfig>(pipelineConfig);

        pipeline->primitive = config->vertexPrimitive;

        pipeline->target = config->targetFb;
        if (!pipeline->target)
            pipeline->target = std::static_pointer_cast<FramebufferOgl, Framebuffer>(GetDefaultFramebuffer());


        /*
        * Shader compilation
        */
        bool vertexFailed = false, pixelFailed = false, linkFailed = false;

        const GLchar* vertexSourcePtr = config->vertexShader->code.c_str();
        const GLchar* pixelSourcePtr = config->pixelShader->code.c_str();

        GLuint vertex, pixel;
        GLint success;
        GLuint program;
        GLchar infoLog[512];

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertexSourcePtr, NULL);
        glCompileShader(vertex);

        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            LOG(ERROR) << "Vertex shader compilation failed: " << std::endl << infoLog;
            vertexFailed = true;
        };

        pixel = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(pixel, 1, &pixelSourcePtr, NULL);
        glCompileShader(pixel);

        glGetShaderiv(pixel, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(pixel, 512, NULL, infoLog);
            LOG(ERROR) << "Fragment shader compilation failed: " << std::endl << infoLog;
            pixelFailed = true;
        };

        if (vertexFailed || pixelFailed)
            throw std::runtime_error("Shaders not compiled");

        program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, pixel);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            LOG(ERROR) << "Shader program not linked: " << std::endl << infoLog;
            linkFailed = true;
        }

        glDeleteShader(vertex);
        glDeleteShader(pixel);

        if (linkFailed)
            throw std::runtime_error("Shader not linked");

        oglState.UseProgram(program);
        pipeline->program = GlResource(program, GlResourceType::Program);
        glCheckError();


        /*
        * Shader parameters
        */
        for (auto&& [name, type] : config->shaderParameters) {
            GLint location = glGetUniformLocation(program, name.c_str());
            if (location < 0) {
                LOG(ERROR) << "Uniform name \"" << name << "\" not found in shader";
            }
            pipeline->shaderParameters.insert({ name,location });
        }
        glCheckError();


        /*
        * Static textures
        */
        size_t usedTextureBlocks = 0;
        for (auto&& [name, texture] : config->textures) {
            GLint location = glGetUniformLocation(program, name.c_str());
            if (location < 0) {
                LOG(ERROR) << "Texture uniform name \"" << name << "\" not found in shader";
            }

            glUniform1i(location, usedTextureBlocks);
            pipeline->staticTextures.push_back(texture);
            usedTextureBlocks++;
        }
        glCheckError();


        /*
        * Vertex attributes
        */
        size_t bufferId = 0;
        for (const auto& buffer : config->vertexBuffers) {
            size_t vertexSize = 0;
            size_t cmdOffset = pipeline->vertexBindCmds.size();
            for (const auto& [name, type, count, instances] : buffer) {
                if (name.empty()) {
                    vertexSize += GalTypeGetSize(type) * count;
                    continue;
                }

                GLint location = glGetAttribLocation(program, name.c_str());
                if (location < 0) {
                    LOG(ERROR) << "Vertex attribute name \"" << name << "\" not found in shader";
                }

                size_t attribSize = GalTypeGetSize(type);

                for (size_t i = 0; i < count; i++) {
                    pipeline->vertexBindCmds.push_back({
                        bufferId,
                        static_cast<size_t>(location + i),
                        GalTypeGetComponentGlType(type),
                        GalTypeGetComponents(type),
                        vertexSize,
                        0,
                        instances,
                        });

                    vertexSize += attribSize;
                }
            }

            for (size_t i = cmdOffset; i < pipeline->vertexBindCmds.size(); i++)
                pipeline->vertexBindCmds[i].offset = vertexSize;

            bufferId++;
        }
        glCheckError();


        return pipeline;
    }

    virtual std::shared_ptr<FramebufferConfig> CreateFramebufferConfig() override {
        auto config = std::make_shared<FramebufferConfigOgl>();
        return std::static_pointer_cast<FramebufferConfig, FramebufferConfigOgl>(config);
    }

    virtual std::shared_ptr<Framebuffer> BuildFramebuffer(std::shared_ptr<FramebufferConfig> config) override {
        auto conf = std::static_pointer_cast<FramebufferConfigOgl, FramebufferConfig>(config);
        auto fb = std::make_shared<FramebufferOgl>();
        
        GLuint newFbo;
        glGenFramebuffers(1, &newFbo);
        fb->fbo = GlResource(newFbo, GlResourceType::Fbo);
        
        oglState.BindFbo(fb->fbo);

        if (conf->depthStencil) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, conf->depthStencil->type, conf->depthStencil->texture, 0);
            fb->depthStencil = std::move(conf->depthStencil);
        }

        for (auto&& [location, texture] : conf->colors) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + location, texture->type, texture->texture, 0);
            fb->colors.emplace_back(std::move(texture));
            fb->attachments.push_back(GL_COLOR_ATTACHMENT0 + location);
        }
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOG(ERROR) << "Framebuffer not completed: " << glCheckFramebufferStatus(GL_FRAMEBUFFER);
        }

        oglState.BindFbo(0);
        glCheckError();

        return std::static_pointer_cast<Framebuffer, FramebufferOgl>(fb);
    }

    virtual std::shared_ptr<Framebuffer> GetDefaultFramebuffer() override {
        if (!fbDefault)
            fbDefault = std::make_shared<FramebufferOgl>();
        fbDefault->fbo = GlResource(0, GlResourceType::None);
        fbDefault->attachments.push_back(GL_COLOR_ATTACHMENT0);
        return std::static_pointer_cast<Framebuffer, FramebufferOgl>(fbDefault);
    }


    virtual std::shared_ptr<ShaderParameters> GetGlobalShaderParameters() override {
        return nullptr;
    }

    virtual std::shared_ptr<Shader> LoadVertexShader(std::string_view code) override {
        auto shader = std::make_shared<ShaderOgl>();
        shader->code = code;
        shader->isVertex = true;
        return std::static_pointer_cast<Shader, ShaderOgl>(shader);
    }

    virtual std::shared_ptr<Shader> LoadPixelShader(std::string_view code) override {
        auto shader = std::make_shared<ShaderOgl>();
        shader->code = code;
        shader->isVertex = false;
        return std::static_pointer_cast<Shader, ShaderOgl>(shader);
    }

};

Impl* Gal::GetImplementation()
{
    if (!impl)
        impl = std::make_unique<ImplOgl>();

    return impl.get();
}
