#include "sdl_wrapper.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <format>
#include <stdexcept>
#include <utility>

Renderer::Renderer(int width, int height, Uint32 window_flags) {
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        throw std::runtime_error(std::format("SDL failed to initialise: {}\n", SDL_GetError()));
    }

    int imgFlags = IMG_INIT_PNG;
    if( !( IMG_Init( imgFlags ) & imgFlags ) ) {
        throw std::runtime_error(std::format("SDL_image failed to initialise: {}\n", IMG_GetError()));
    }

    if (SDL_CreateWindowAndRenderer(width, height, window_flags, &window, &renderer)) {
        throw std::runtime_error(std::format("Failed to create window or renderer: {}\n", SDL_GetError()));
    }
}

Renderer::~Renderer() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void Renderer::draw_texture(const Texture& texture, int x, int y) {
    if (texture.inner() == nullptr) {
        throw new std::runtime_error("Tried to render a null texture to the screen");
    }

    SDL_Rect destRect;
    destRect.x = x;
    destRect.y = y;
    destRect.w = texture.getWidth();
    destRect.h = texture.getHeight();
    SDL_RenderCopy(renderer, texture.inner(), nullptr, &destRect);
}

void Renderer::draw_texture(const Texture& texture, const std::optional<SDL_Rect>& srcRect, const SDL_Rect& destRect) {
    if (texture.inner() == nullptr) {
        throw new std::runtime_error("Tried to render a null texture to the screen");
    }

    const SDL_Rect *src = nullptr;
    if (srcRect) {
        src = &srcRect.value();
    }

    SDL_RenderCopy(renderer, texture.inner(), src, &destRect);
}

void Renderer::set_draw_colour(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void Renderer::clear() {
    SDL_RenderClear(renderer);
}

void Renderer::present() {
    SDL_RenderPresent(renderer);
}

Texture::Texture(Renderer& renderer, std::string path) {
    SDL_Surface *surface = IMG_Load(path.c_str());

    if (surface == nullptr) {
        throw std::runtime_error(std::format("Couldn't load surface from path \"{}\": {}", path, IMG_GetError()));
    }

    width = surface->w;
    height = surface->h;
    texture = SDL_CreateTextureFromSurface(renderer.inner(), surface);

    if (texture == nullptr) {
        throw std::runtime_error(std::format("Couldn't convert surface to texture: {}", SDL_GetError()));
    }

    SDL_FreeSurface(surface);
}

Texture::Texture(Texture&& t) noexcept
    : texture(std::exchange(t.texture, nullptr)), width(t.width), height(t.height)
{}

Texture& Texture::operator=(Texture&& t) noexcept {
    std::swap(t.texture, texture);
    width = t.width;
    height = t.height;
    return *this;
}

Texture::~Texture() {
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
    }
}

int Texture::getWidth() const {
    return width;
}

int Texture::getHeight() const {
    return height;
}
