#include "Gal.hpp"

#include <easylogging++.h>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include "Utility.hpp"

using namespace Gal;

class ImplOgl;
class FramebufferDefaultOgl;
class ShaderOgl;

std::unique_ptr<ImplOgl> impl;
std::shared_ptr<FramebufferDefaultOgl> fbDefault;

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


class ShaderOgl : public Shader {
public:
    bool isVertex = true;
    std::string code;
};

class BufferBindingOgl : public BufferBinding {
public:
    BufferBindingOgl(size_t id) : bufferId(id) {}

    const size_t bufferId;

    static constexpr size_t indexValue = (std::numeric_limits<size_t>::max)(); //parenthess for windows' max macro
};

class BufferOgl : public Buffer {
public:
    GLuint vbo;

    virtual void SetData(std::vector<std::byte>&& data) override {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size(), data.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glCheckError();
    }
};

class TextureConfigOgl : public TextureConfig {
public:

    Format format;
    size_t width = 1, height = 1, depth = 1;
    bool interpolateLayers = false;

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

class TextureOgl : public Texture {
public:

    GLenum type;
    GLuint texture;
    Format format;
    size_t width, height, depth;

    virtual void SetData(std::vector<std::byte>&& data, size_t mipLevel = 0) override {
        size_t expectedSize = width * height * depth * GalFormatGetSize(format);
        if (data.size() != expectedSize)
            throw std::logic_error("Size of data is not valid for this texture");

        glBindTexture(type, texture);
        glCheckError();

        switch (type) {
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
            glTexImage2D(type, mipLevel, GalFormatGetGlInternalFormat(format), width, height, 0, GalFormatGetGlFormat(format), GalFormatGetGlType(format), data.data());
            break;
        case GL_TEXTURE_3D:
        case GL_PROXY_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY:
        case GL_PROXY_TEXTURE_2D_ARRAY:
            glTexImage3D(type, mipLevel, GalFormatGetGlInternalFormat(format), width, height, depth, 0, GalFormatGetGlFormat(format), GalFormatGetGlType(format), data.data());
            break;
        default:
            throw std::runtime_error("Unknown texture type");
        }

        glCheckError();

        glBindTexture(type, 0);
    }

};

class PipelineConfigOgl : public PipelineConfig {
public:
    std::shared_ptr<ShaderOgl> vertexShader, pixelShader;
    std::map<std::string, Type> shaderParameters;
    std::shared_ptr<Framebuffer> targetFb;
    std::vector<std::vector<VertexAttribute>> vertexBuffers;
    Primitive vertexPrimitive = Primitive::Triangle;
public:
    virtual void SetVertexShader(std::shared_ptr<Shader> shader) override {
        vertexShader = std::static_pointer_cast<ShaderOgl,Shader>(shader);
    }

    virtual void SetPixelShader(std::shared_ptr<Shader> shader) override {
        pixelShader = std::static_pointer_cast<ShaderOgl, Shader>(shader);
    }

    virtual void AddShaderParameter(std::string_view name, Type type) override {
        shaderParameters.emplace(std::string(name), type);
    }

    virtual void SetTarget(std::shared_ptr<Framebuffer> target) override {
        targetFb = target;
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

class PipelineInstanceOgl : public PipelineInstance {
public:
    GLuint vao;
    bool useIndex = false;
    Primitive primitive;
    size_t instances = 0;

    virtual void Activate() override {
        glBindVertexArray(vao);
        glCheckError();
    }

    virtual void Render(size_t offset = 0, size_t count = -1) override {
        GLenum vertexMode;
        switch (primitive) {
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

        glCheckError();
    }

    virtual void SetInstancesCount(size_t count) override {
        instances = count;
    }
};

class PipelineOgl : public Pipeline {
public:
    std::map<std::string, size_t> shaderParameters;
    GLuint program;
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
    
    virtual void Activate() override {
        glUseProgram(program);
        glCheckError();
    }

    virtual void SetDynamicTexture(std::string_view name, std::shared_ptr<Texture> texture) override {
        Activate();
        glActiveTexture(GL_TEXTURE0);
        auto tex = std::static_pointer_cast<TextureOgl>(texture);
        glBindTexture(tex->type, tex->texture);
        SetShaderParameter(name, 0);
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

        glGenVertexArrays(1, &instance->vao);
        glBindVertexArray(instance->vao);
        glCheckError();

        for (const auto& cmd : vertexBindCmds) {
            glBindBuffer(GL_ARRAY_BUFFER, bufferBindingId.find(cmd.bufferId)->second);
            glCheckError();
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
            
            glCheckError();
            glEnableVertexAttribArray(cmd.location);
            glCheckError();
            if (cmd.instances) {
                glVertexAttribDivisor(cmd.location, cmd.instances);
                glCheckError();
            }
        }

        if (indexBuffer != BufferBindingOgl::indexValue) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            instance->useIndex = true;
        }

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glCheckError();

        return instance;
    }

    virtual void SetShaderParameter(std::string_view name, float value) override {
        Activate();
        glUniform1f(shaderParameters.at(std::string(name)), value);
        glCheckError();
    }

    virtual void SetShaderParameter(std::string_view name, double value) override {
        Activate();
        glUniform1d(shaderParameters.at(std::string(name)), value);
        glCheckError();
    }

    virtual void SetShaderParameter(std::string_view name, int8_t value) override {
        Activate();
        glUniform1i(shaderParameters.at(std::string(name)), value);
        glCheckError();
    }

    virtual void SetShaderParameter(std::string_view name, int16_t value) override {
        Activate();
        glUniform1i(shaderParameters.at(std::string(name)), value);
        glCheckError();
    }

    virtual void SetShaderParameter(std::string_view name, int32_t value) override {
        Activate();
        glUniform1i(shaderParameters.at(std::string(name)), value);
        glCheckError();
    }

    virtual void SetShaderParameter(std::string_view name, uint8_t value) override {
        Activate();
        glUniform1ui(shaderParameters.at(std::string(name)), value);
        glCheckError();
    }

    virtual void SetShaderParameter(std::string_view name, uint16_t value) override {
        Activate();
        glUniform1ui(shaderParameters.at(std::string(name)), value);
        glCheckError();
    }

    virtual void SetShaderParameter(std::string_view name, uint32_t value) override {
        Activate();
        glUniform1ui(shaderParameters.at(std::string(name)), value);
        glCheckError();
    }

    virtual void SetShaderParameter(std::string_view name, glm::vec2 value) override {
        Activate();
        glUniform2f(shaderParameters.at(std::string(name)), value.x, value.y);
        glCheckError();
    }

    virtual void SetShaderParameter(std::string_view name, glm::uvec2 value) override {
        Activate();
        glUniform2ui(shaderParameters.at(std::string(name)), value.x, value.y);
        glCheckError();
    }

    virtual void SetShaderParameter(std::string_view name, glm::vec3 value) override {
        Activate();
        glUniform3f(shaderParameters.at(std::string(name)), value.x, value.y, value.z);
        glCheckError();
    }

    virtual void SetShaderParameter(std::string_view name, glm::vec4 value) override {
        Activate();
        glUniform4f(shaderParameters.at(std::string(name)), value.x, value.y, value.z, value.w);
        glCheckError();
    }

    virtual void SetShaderParameter(std::string_view name, glm::mat4 value) override {
        Activate();
        glUniformMatrix4fv(shaderParameters.at(std::string(name)), 1, GL_FALSE, glm::value_ptr(value));
        glCheckError();
    }
};

class ImplOgl : public Impl {

public:

    virtual void Init() override {
        LOG(INFO) << "Initalizing Gal:OpenGL...";
        LOG(INFO) << "Initializing GLEW";
        glewExperimental = GL_TRUE;
        GLenum glewStatus = glewInit();
        glCheckError();
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
    }

    virtual void DeInit() override {
        LOG(INFO) << "Destroying Gal:OpenGL...";
    }

    virtual void Cleanup() override {

    }

    virtual void SetScissor(size_t x = 0, size_t y = 0, size_t width = 0, size_t height = 0) override {
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y, width, height);
    }

    virtual void SetScissor(bool enabled) override {
        if (enabled)
            glEnable(GL_SCISSOR_TEST);
        else
            glDisable(GL_SCISSOR_TEST);
    }


    virtual std::shared_ptr<Buffer> CreateBuffer() override {
        auto buff = std::make_shared<BufferOgl>();
        glGenBuffers(1, &buff->vbo);
        buff->SetData({});
        glCheckError();
        return std::static_pointer_cast<Buffer, BufferOgl>(buff);
    }


    virtual std::shared_ptr<TextureConfig> CreateTexture2DConfig(size_t width, size_t height, Format format) override {
        auto config = std::make_shared<TextureConfigOgl>();

        config->width = width;
        config->height = height;
        config->depth = 1;
        config->format = format;

        return std::static_pointer_cast<TextureConfig, TextureConfigOgl>(config);
    }

    virtual std::shared_ptr<TextureConfig> CreateTexture3DConfig(size_t width, size_t height, size_t depth, bool interpolateLayers, Format format) override {
        auto config = std::make_shared<TextureConfigOgl>();

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

        texture->type = GL_TEXTURE_2D;
        texture->format = texConfig->format;
        texture->width = texConfig->width;
        texture->height = texConfig->height;
        texture->depth = texConfig->depth;

        glGenTextures(1, &texture->texture);
        glCheckError();
        glBindTexture(texture->type, texture->texture);

        glTexParameteri(texture->type, GL_TEXTURE_MIN_FILTER, GalFilteringGetGlType(texConfig->min));
        glTexParameteri(texture->type, GL_TEXTURE_MAG_FILTER, GalFilteringGetGlType(texConfig->max));
        glTexParameteri(texture->type, GL_TEXTURE_WRAP_S, GalWrappingGetGlType(texConfig->wrap));
        glTexParameteri(texture->type, GL_TEXTURE_WRAP_T, GalWrappingGetGlType(texConfig->wrap));
        glCheckError();

        glBindTexture(texture->type, 0);
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

        //Shader compilation

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

        glUseProgram(program);

        glCheckError();

        pipeline->program = program;

        //Shader parameters

        for (auto&& [name, type] : config->shaderParameters) {
            GLint location = glGetUniformLocation(program, name.c_str());
            if (location < 0) {
                glDeleteProgram(program);
                LOG(ERROR) << "Uniform name \"" << name << "\" not found in shader";
                throw std::runtime_error("Invalid uniform");
            }
            switch (type) {
            case Type::Vec2:
                glUniform2f(location, 0.0f, 0.0f);
                break;
            case Type::Vec2u8:
            case Type::Vec2u16:
            case Type::Vec2u32:
                glUniform2ui(location, 0, 0);
                break;
            case Type::Vec4u8:
                glUniform4ui(location, 0, 0, 0, 0);
                break;
            }
            pipeline->shaderParameters.insert({ name,location });
        }

        //Vertex attributes

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
                    glDeleteProgram(program);
                    LOG(ERROR) << "Vertex attribute name \"" << name << "\" not found in shader";
                    throw std::runtime_error("Invalid attribute");
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
        return nullptr;
    }

    virtual std::shared_ptr<Framebuffer> BuildFramebuffer(std::shared_ptr<FramebufferConfig> config) override {
        return nullptr;
    }

    virtual std::shared_ptr<Framebuffer> GetDefaultFramebuffer() override {
        if (!fbDefault)
            fbDefault = std::make_shared<FramebufferDefaultOgl>();
        return std::static_pointer_cast<Framebuffer, FramebufferDefaultOgl>(fbDefault);
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

class FramebufferDefaultOgl : public Framebuffer {
    size_t vpX, vpY, vpW, vpH;
public:

    virtual void Clear() override {
        GLbitfield clearBits = 0;
        clearBits |= GL_COLOR_BUFFER_BIT;
        clearBits |= GL_DEPTH_BUFFER_BIT;
        clearBits |= GL_STENCIL_BUFFER_BIT;
        glClear(clearBits);
    }

    virtual void SetViewport(size_t x, size_t y, size_t w, size_t h) override {
        vpX = x;
        vpY = y;
        vpW = w;
        vpH = h;
        glViewport(x, y, w, h);
    }
};

Impl* Gal::GetImplementation()
{
    if (!impl)
        impl = std::make_unique<ImplOgl>();

    return impl.get();
}
