#pragma once

#include <map>
#include <SDL.h>

struct MouseState {
    MouseState() : position(std::pair(-1, -1)) {}

    // Call this every frame *before* handling input
    void update();
    // Update the state of the mouse based on an input event
    void handle_input(const SDL_Event& e);

    bool is_pressed(Uint8 button);
    bool is_just_pressed(Uint8 button);
    bool is_just_released(Uint8 button);

    std::pair<int, int> pos();
    
private:
    std::map<Uint8, std::pair<bool, bool>> buttons;
    std::pair<int, int> position;
};
