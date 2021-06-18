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

RmlRenderInterface::RmlRenderInterface(RenderState& renderState) : State(&renderState) {
    glGenVertexArrays(1, &Vao);
    glBindVertexArray(Vao);
    glCheckError();

    glGenBuffers(1, &Ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STREAM_DRAW);
    glCheckError();

    glGenBuffers(1, &Vbo);
    glBindBuffer(GL_ARRAY_BUFFER, Vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STREAM_DRAW);
    glCheckError();
    
    {
        //Vertex position (2 float)
        GLuint PosAttribPos = 0;
        glVertexAttribPointer(PosAttribPos, 2, GL_FLOAT, GL_FALSE, 20, (void*)0);
        glEnableVertexAttribArray(PosAttribPos);

        //Vertex colour (4 uint8 RGBA)
        GLuint ColAttribPos = 1;
        glVertexAttribIPointer(ColAttribPos, 4, GL_UNSIGNED_BYTE, 20, (void*)8);
        glEnableVertexAttribArray(ColAttribPos);

        //Vertex tex_coord (2 float)
        GLuint TexAttribPos = 2;
        glVertexAttribPointer(TexAttribPos, 2, GL_FLOAT, GL_FALSE, 20, (void*)12);
        glEnableVertexAttribArray(TexAttribPos);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glCheckError();
}

RmlRenderInterface::~RmlRenderInterface() {
    glDeleteVertexArrays(1, &Vao);
    glDeleteBuffers(1, &Vbo);
    glDeleteBuffers(1, &Ebo);
    glCheckError();
}

void RmlRenderInterface::RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture, const Rml::Vector2f& translation) {
    if (texture) {
        AssetManager::GetAsset<AssetShader>("/altcraft/shaders/rmltex")->shader->Activate();
        AssetManager::GetAsset<AssetShader>("/altcraft/shaders/rmltex")->shader->SetUniform("translation", glm::vec2(translation.x, translation.y));
        glBindTexture(GL_TEXTURE_2D, texture);
    } else {
        AssetManager::GetAsset<AssetShader>("/altcraft/shaders/rml")->shader->Activate();
        AssetManager::GetAsset<AssetShader>("/altcraft/shaders/rml")->shader->SetUniform("translation", glm::vec2(translation.x, translation.y));
    }   
    glCheckError();

    glBindVertexArray(Vao);
    glCheckError();

    glBindBuffer(GL_ARRAY_BUFFER, Vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(Rml::Vertex), vertices, GL_STREAM_DRAW);
    glCheckError();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(int), indices, GL_STREAM_DRAW);
    glCheckError();

    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
    glCheckError();
    glBindVertexArray(0);
}

void RmlRenderInterface::EnableScissorRegion(bool enable) {

}

void RmlRenderInterface::SetScissorRegion(int x, int y, int width, int height) {

}

bool RmlRenderInterface::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) {
    return false;
}

bool RmlRenderInterface::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) {
    int mipLevelCount = 1;
    glActiveTexture(GL_TEXTURE0);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glCheckError();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, source_dimensions.x, source_dimensions.y, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, source);
    glCheckError();

    texture_handle = texture;
    return true;
}

void RmlRenderInterface::ReleaseTexture(Rml::TextureHandle texture) {
    GLuint textures = texture;
    glDeleteTextures(1, &textures);
    glCheckError();
}

void RmlRenderInterface::Update(unsigned int windowWidth, unsigned int windowHeight) {
    AssetManager::GetAsset<AssetShader>("/altcraft/shaders/rml")->shader->Activate();
    AssetManager::GetAsset<AssetShader>("/altcraft/shaders/rml")->shader->SetUniform("viewportSize", windowWidth, windowHeight);
    glCheckError();
    AssetManager::GetAsset<AssetShader>("/altcraft/shaders/rmltex")->shader->Activate();
    AssetManager::GetAsset<AssetShader>("/altcraft/shaders/rmltex")->shader->SetUniform("viewportSize", windowWidth, windowHeight);
    AssetManager::GetAsset<AssetShader>("/altcraft/shaders/rmltex")->shader->SetUniform("fontTexture", 0);
    glCheckError();
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
    readed = _min(handles[file].assetPtr->data.size() - handles[file].filePos, size);
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
