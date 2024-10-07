#pragma once

#include <array>
#include <SDL.h>
#include <memory>
#include <vector>
#include "sdl_wrapper.hpp"
#include "ai/ai.hpp"
#include "cards.hpp"
#include "input.hpp"

struct HeldCard {
    Card c;
    // If this stackCoord is nullopt, then the card is from the pile.
    std::optional<std::pair<int, int>> stackCoord;
    std::pair<int, int> mouseOffset;

    HeldCard() {}
    HeldCard(Card c, std::pair<int, int> mouseOffset)
        : c(c), stackCoord(), mouseOffset(mouseOffset)
    {}

    HeldCard(Card c, std::pair<int, int> mouseOffset, std::pair<int, int> stackCoord)
        : c(c), stackCoord(stackCoord), mouseOffset(mouseOffset)
    {}
};

struct Game {
    Game(int draw);
    Game(int draw, std::unique_ptr<SolitaireAI> ai);

    void setup_game();
    void run();
    void run_ai();
    bool is_solved();

private:
    Renderer renderer;

    // sprites for the game
    Texture cardTexture;
    Texture cardOutline;

    // The ai, if any
    std::unique_ptr<SolitaireAI> ai;
    float aiMoveTimer = 0;
    bool useAi;

    // Event tracking
    MouseState mouse;

    // Game model
    std::array<std::vector<Card>, 7> playfield;
    std::array<std::vector<Card>, 4> aces;
    std::vector<Card> stock;
    std::vector<Card> pile;
    std::optional<HeldCard> held;
    int cardDraw;

    void update(float dt);
    void render();
    void render_card(const Card& card, int x, int y);
    void render_card_back(int x, int y);
    void render_card_outline(int x, int y);

    std::optional<HeldCard> get_hovered_card(int tolerance);
    std::optional<int> get_hovered_aces_id(int tolerance);
    std::optional<int> get_hovered_empty_id(int tolerance);

    const Card& get_card(CardSource src, std::pair<int, int> coord);
    std::vector<Card> pop_cards(CardSource src, std::pair<int, int> coord);

    void pop_held_cards();

    int stack_height(int i);

    void deal_or_reset_stock();
};
