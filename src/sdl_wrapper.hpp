#pragma once

#include <SDL.h>
#include <optional>
#include <string>

struct Renderer;
struct Texture;

/// Wrapper for the renderer and window.
struct Renderer {
    Renderer(int window_width, int window_height, Uint32 window_flags);
    Renderer(const Renderer& r) = delete;
    Renderer& operator=(const Renderer& r) = delete;
    ~Renderer();

    // Returns the wrapped SDL_Renderer
    // be careful with it, okay?
    SDL_Renderer* inner() { return renderer; }

    void set_draw_colour(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void clear();
    void present();

    /// Draw the texture directly onto the screen at a certain position
    void draw_texture(const Texture& texture, int x, int y);
    /// Draw the texture with full control over src and dst (See SDL_RenderCopy)
    void draw_texture(const Texture& texture, const std::optional<SDL_Rect>& src, const SDL_Rect& dest);

private:
    SDL_Renderer* renderer;
    SDL_Window* window;
};

/// Wrapper for an sdl texture on the GPU
struct Texture {
    Texture() : texture(nullptr) {}
    Texture(Renderer& renderer, std::string path);
    // We can't copy an SDL_Texture, only move. If you want to share a Texture around use a
    // shared_ptr.
    Texture(const Texture& t) = delete;
    Texture& operator=(const Texture& t) = delete;
    Texture(Texture&& t) noexcept;
    Texture& operator=(Texture&& t) noexcept;
    
    ~Texture();

    // Returns the wrapped SDL_Texture
    SDL_Texture* inner() const { return texture; };

    // These getters return the width and height of the entire texture in pixels
    int getWidth() const;
    int getHeight() const;
    
private:
    SDL_Texture* texture;

    int width;
    int height;
};
