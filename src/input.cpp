#include <SDL.h>
#include <utility>
#include "input.hpp"

void MouseState::handle_input(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
        bool pressed = e.button.state == SDL_PRESSED;
        // This line is impossible to read so here's the explanation
        // If there is no entry in the map we insert a new one mapping { e.button.button => (false, pressed) }
        // Otherwise, we assign the second element of the value to `pressed`
        if (auto pair = buttons.insert({ e.button.button, std::pair(false, pressed) }); !pair.second) {
            pair.first->second.second = pressed;
        }
    } else if (e.type == SDL_MOUSEMOTION) {
        SDL_GetMouseState(&position.first, &position.second);
    }
}

void MouseState::update() {
    for (auto i = buttons.begin(); i != buttons.end(); i++) {
        std::pair<bool, bool>& state = i->second;
        state.first = state.second;
    }
}

bool MouseState::is_pressed(Uint8 button) {
    if (auto pair = buttons.find(button); pair != buttons.end()) {
        return pair->second.second;
    } else {
        return false;
    }
}

bool MouseState::is_just_pressed(Uint8 button) {
    if (auto pair = buttons.find(button); pair != buttons.end()) {
        return !pair->second.first && pair->second.second;
    } else {
        return false;
    }
}

bool MouseState::is_just_released(Uint8 button) {
    if (auto pair = buttons.find(button); pair != buttons.end()) {
        return pair->second.first && !pair->second.second;
    } else {
        return false;
    }
}

std::pair<int, int> MouseState::pos() {
    return position;
}
