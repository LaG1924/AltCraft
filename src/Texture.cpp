#include "Texture.hpp"

#include <SDL.h>
#include <SDL_image.h>


bool IsImgInitialized = false;

Texture::Texture(std::string filename, GLenum textureWrapping, GLenum textureFiltering) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    //Texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapping);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapping);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //Initialize SDL2_image
    if (!IsImgInitialized) {
        IsImgInitialized = true;
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags))
        {
            throw std::runtime_error("IMG Not initialized: " + std::string(IMG_GetError()));
        }
    }

    //Load image
    SDL_Surface *image = IMG_Load(filename.c_str());
    if (!image)
        throw std::runtime_error("Image not loaded: " + std::string(IMG_GetError()));

    if (image->format->BytesPerPixel != 4)
        throw std::runtime_error("PNG format is not RGBA");

    //Flip surface
    SDL_LockSurface(image);

    {
        int pitch = image->pitch;
        int height = image->h;
        void* image_pixels = image->pixels;

        int index;
        void* temp_row;
        int height_div_2;

        temp_row = (void *)malloc(pitch);
        if (!temp_row)
            throw std::runtime_error("Not enough memory for texture flipping");
        
        height_div_2 = (int)(height * .5);
        for (index = 0; index < height_div_2; index++) {
            memcpy((Uint8 *)temp_row,
                (Uint8 *)(image_pixels)+
                pitch * index,
                pitch);

            memcpy(
                (Uint8 *)(image_pixels)+
                pitch * index,
                (Uint8 *)(image_pixels)+
                pitch * (height - index - 1),
                pitch);
            memcpy(
                (Uint8 *)(image_pixels)+
                pitch * (height - index - 1),
                temp_row,
                pitch);
        }
        free(temp_row);
    }

    //Creating texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w,image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 (GLvoid *) image->pixels);
    SDL_UnlockSurface(image);

    //glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(image);
}

Texture::~Texture() {
    glDeleteTextures(1, &texture);
}
