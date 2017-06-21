#include <graphics/Texture.hpp>

Texture::Texture(std::string filename, GLenum textureWrapping, GLenum textureFiltering) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    //Texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapping);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFiltering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //Image load
    sf::Image image;
    if (!image.loadFromFile(filename)) {
        LOG(ERROR) << "Can't open image " << filename;
        throw 201;
    }
    if (image.getPixelsPtr() == nullptr) {
        LOG(ERROR) << "Image data is corrupted!";
        throw 202;
    }
    image.flipVertically();


    //Creating texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 (GLvoid *) image.getPixelsPtr());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

}

Texture::~Texture() {
    glDeleteTextures(1, &texture);
}
