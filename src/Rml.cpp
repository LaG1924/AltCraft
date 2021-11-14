#include "Rml.hpp"

#include <easylogging++.h>

#include "AssetManager.hpp"
#include "Shader.hpp"
#include "Utility.hpp"

double RmlSystemInterface::GetElapsedTime() {
	return totalTime;
}

bool RmlSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message) {
    switch (type) {
    case Rml::Log::Type::LT_ALWAYS:
        LOG(ERROR) << message;
        break;
    case Rml::Log::Type::LT_ERROR:
        LOG(ERROR) << message;
        break;
    case Rml::Log::Type::LT_ASSERT:
        LOG(ERROR) << message;
        break;
    case Rml::Log::Type::LT_WARNING:
        LOG(WARNING) << message;
        break;
    case Rml::Log::Type::LT_INFO:
        LOG(INFO) << message;
        break;
    case Rml::Log::Type::LT_DEBUG:
        LOG(DEBUG) << message;
        break;
    case Rml::Log::Type::LT_MAX:
        LOG(DEBUG) << message;
        break;
    }
    return true;
}

void RmlSystemInterface::SetClipboardText(const Rml::String& text) {
    clipboard = text;
}

void RmlSystemInterface::GetClipboardText(Rml::String& text) {
    text = clipboard;
}

RmlRenderInterface::RmlRenderInterface(RenderState& renderState) {
    auto gal = Gal::GetImplementation();
    auto pipelineConfig = gal->CreatePipelineConfig();
    pipelineConfig->AddShaderParameter("viewportSize", Gal::Type::Vec2u32);
    pipelineConfig->AddShaderParameter("translation", Gal::Type::Vec2);
    pipelineConfig->SetTarget(gal->GetDefaultFramebuffer());

    auto vertAsset = AssetManager::GetAssetByAssetName("/altcraft/shaders/vert/rml");
    std::string vertSource((char*)vertAsset->data.data(), (char*)vertAsset->data.data() + vertAsset->data.size());
    auto pixelAsset = AssetManager::GetAssetByAssetName("/altcraft/shaders/frag/rml");
    std::string pixelSource((char*)pixelAsset->data.data(), (char*)pixelAsset->data.data() + pixelAsset->data.size());
    pipelineConfig->SetVertexShader(gal->LoadVertexShader(vertSource));
    pipelineConfig->SetPixelShader(gal->LoadPixelShader(pixelSource));

    auto vertBuffBind = pipelineConfig->BindVertexBuffer({
        {"pos", Gal::Type::Vec2},
        {"color", Gal::Type::Vec4u8},
        {"", Gal::Type::Vec2}, //it's not used in shader, so driver optimizes it away
        });

    auto indexBuffBind = pipelineConfig->BindIndexBuffer();

    pipeline = gal->BuildPipeline(pipelineConfig);

    vertexBuffer = gal->CreateBuffer();

    indexBuffer = gal->CreateBuffer();

    pipelineInstance = pipeline->CreateInstance({
        {vertBuffBind, vertexBuffer},
        {indexBuffBind, indexBuffer},
        });

    glCheckError();

    auto texturePipelineConfig = gal->CreatePipelineConfig();
    texturePipelineConfig->AddShaderParameter("viewportSize", Gal::Type::Vec2u32);
    texturePipelineConfig->AddShaderParameter("translation", Gal::Type::Vec2);
    texturePipelineConfig->AddShaderParameter("fontTexture", Gal::Type::Int32);
    texturePipelineConfig->SetTarget(gal->GetDefaultFramebuffer());

    auto texturePixelAsset = AssetManager::GetAssetByAssetName("/altcraft/shaders/frag/rmltex");
    std::string texturePixelSource((char*)texturePixelAsset->data.data(), (char*)texturePixelAsset->data.data() + texturePixelAsset->data.size());
    texturePipelineConfig->SetVertexShader(gal->LoadVertexShader(vertSource));
    texturePipelineConfig->SetPixelShader(gal->LoadPixelShader(texturePixelSource));

    auto texVertBuffBind = texturePipelineConfig->BindVertexBuffer({
        {"pos", Gal::Type::Vec2},
        {"color", Gal::Type::Vec4u8},
        {"tex_coord", Gal::Type::Vec2},
        });

    auto texIndexBuffBind = texturePipelineConfig->BindIndexBuffer();

    texPipeline = gal->BuildPipeline(texturePipelineConfig);

    texPipelineInstance = texPipeline->CreateInstance({
        {texVertBuffBind, vertexBuffer},
        {texIndexBuffBind, indexBuffer},
        });
    glCheckError();
}

RmlRenderInterface::~RmlRenderInterface() {
    glCheckError();
}

void RmlRenderInterface::RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation) {
    indexBuffer->SetData({ reinterpret_cast<std::byte*>(indices), reinterpret_cast<std::byte*>(indices + num_indices) });
    vertexBuffer->SetData({ reinterpret_cast<std::byte*>(vertices), reinterpret_cast<std::byte*>(vertices + num_vertices) });
    glCheckError();
    
    auto tex = textures.find(texture);
    if (tex != textures.end()) {
        texPipeline->Activate();
        glCheckError();
        texPipeline->SetShaderParameter("translation", glm::vec2(translation.x, translation.y));
        glCheckError();
        texPipeline->SetDynamicTexture("fontTexture", tex->second);
        glCheckError();
        texPipelineInstance->Activate();
        glCheckError();
        texPipelineInstance->Render(0, num_indices);
    } else {
        pipeline->Activate();
        glCheckError();
        pipeline->SetShaderParameter("translation", glm::vec2(translation.x, translation.y));
        glCheckError();
        pipelineInstance->Activate();
        glCheckError();
        pipelineInstance->Render(0, num_indices);
    }
    glCheckError();
}

void RmlRenderInterface::EnableScissorRegion(bool enable) {
    if (enable)
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);
}

void RmlRenderInterface::SetScissorRegion(int x, int y, int width, int height) {
    glScissor(x, vpHeight - (y + height), width, height);
    glCheckError();
}

bool RmlRenderInterface::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) {
    return false;
}

bool RmlRenderInterface::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) {
    glCheckError();
    size_t textureId = textures.empty() ? 1 : textures.rbegin()->first + 1;
    auto gal = Gal::GetImplementation();
    auto textureConfig = gal->CreateTexture2DConfig(source_dimensions.x, source_dimensions.y, Gal::Format::R8G8B8A8);
    auto texture = gal->BuildTexture(textureConfig);
    texture->SetData({ reinterpret_cast<const std::byte*>(source),reinterpret_cast<const std::byte*>(source + (source_dimensions.x * source_dimensions.y) * 4) });
    textures.insert({ textureId,texture });
    texture_handle = textureId;
    glCheckError();

    return true;
}

void RmlRenderInterface::ReleaseTexture(Rml::TextureHandle texture) {
    GLuint textures = texture;
    glDeleteTextures(1, &textures);
    glCheckError();
}

void RmlRenderInterface::Update(unsigned int windowWidth, unsigned int windowHeight) {

    glCheckError();
    
    pipeline->SetShaderParameter("viewportSize", glm::uvec2(windowWidth, windowHeight));
    texPipeline->SetShaderParameter("viewportSize", glm::uvec2(windowWidth, windowHeight));
    texPipeline->SetShaderParameter("fontTexture", 0);

    glCheckError();

    vpWidth = windowWidth;
    vpHeight = windowHeight;
}

Rml::FileHandle RmlFileInterface::Open(const Rml::String& path) {
    Rml::FileHandle fileId = handles.rbegin() != handles.rend() ? handles.rbegin()->first + 1 : 1;
    while (handles.find(fileId) != handles.end())
        fileId++;

    AssetHandle handle;
    handle.fileName = path;
    std::string assetName = path;
    if (*assetName.begin() != '/')
        assetName = "/" + assetName;
    handle.assetPtr = AssetManager::GetAssetByAssetName(assetName);
    handle.filePos = 0;

    if (handle.assetPtr != nullptr)
        handles.insert(std::make_pair(fileId, handle));
    else
        fileId = 0;
    return fileId;
}

void RmlFileInterface::Close(Rml::FileHandle file) {
    handles.erase(file);
}

size_t RmlFileInterface::Read(void* buffer, size_t size, Rml::FileHandle file) {
    size_t readed = 0;
    readed = _min((size_t)(handles[file].assetPtr->data.size() - handles[file].filePos), size);
    std::memcpy(buffer, handles[file].assetPtr->data.data() + handles[file].filePos, readed);
    handles[file].filePos += readed;
    return readed;
}

bool RmlFileInterface::Seek(Rml::FileHandle file, long offset, int origin) {
    unsigned long long base = 0;
    if (origin == SEEK_CUR)
        base = handles[file].filePos;
    else if (origin == SEEK_END)
        base = handles[file].assetPtr->data.size();
    handles[file].filePos = base + offset;
    return true;
}

size_t RmlFileInterface::Tell(Rml::FileHandle file) {
    return handles[file].filePos;
}
