#include <SDL.h>
#include <SDL_image.h>
#include <algorithm>
#include <random>
#include <print>
#include <stdexcept>
#include <variant>

#include "game.hpp"
#include "cards.hpp"
#include "ai/ai.hpp"
#include "utils.hpp"

const int WINDOW_WIDTH = 900;
const int WINDOW_HEIGHT = 800;

const int CARD_TILE_WIDTH = 64;
const int CARD_TILE_HEIGHT = 64;
const int CARD_TILE_OFFSET_X = 11;
const int CARD_TILE_OFFSET_Y = 2;
const int CARD_SPRITE_WIDTH = 42;
const int CARD_SPRITE_HEIGHT = 60;
const int CARD_UPSCALE = 2;

const int PLAYFIELD_START_X = 100;
const int PLAYFIELD_START_Y = 200;
const int PLAYFIELD_CARD_DX = 110;
const int PLAYFIELD_UP_CARD_DY = 30;
const int PLAYFIELD_DOWN_CARD_DY = 15;

const int STOCK_X = 100;
const int PILE_X = 210;
const int PILE_DX = 30;
const int STOCK_PILE_Y = 50;

const int ACES_X = 430;
const int ACES_DX = 110;

const float AI_MOVE_TIME = 1./5.;

Game::Game(int draw) :
    renderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN),
    cardTexture(renderer, "assets/cards.png"),
    cardOutline(renderer, "assets/outline.png"),
    useAi(false),
    cardDraw(draw)
{}

Game::Game(int draw, std::unique_ptr<SolitaireAI> ai) :
    renderer(WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN),
    cardTexture(renderer, "assets/cards.png"),
    cardOutline(renderer, "assets/outline.png"),
    ai(std::move(ai)),
    aiMoveTimer(0),
    useAi(true),
    cardDraw(draw)
{}

bool is_hovering_card(std::pair<int, int> mousePos, std::pair<int, int> cardPos, int tolerance) {
    int mpx = mousePos.first;
    int mpy = mousePos.second;
    int x = cardPos.first;
    int y = cardPos.second;

    return mpx >= x - tolerance && mpy >= y - tolerance
        && mpx <= x + CARD_SPRITE_WIDTH * CARD_UPSCALE + tolerance
        && mpy <= y + CARD_SPRITE_HEIGHT * CARD_UPSCALE + tolerance;
}

void Game::setup_game() {
    for (int i = 0; i < 7; i++) {
        playfield.at(i).clear();
    }

    pile.clear();
    stock.clear();

    for (int i = 0; i < 4; i++) {
        aces.at(i).clear();
    }

    held = std::nullopt;

    // populate the deck and shuffle it
    stock.reserve(52);
    for (int s = 0; s < 4; s++) {
        for (int v = 0; v < 13; v++) {
            Suit suit = static_cast<Suit>(s);
            Value value = static_cast<Value>(v);

            stock.push_back(Card(value, suit));
        }
    }

    std::mt19937 rand(std::random_device{}());
    std::shuffle(stock.begin(), stock.end(), rand);

    // Deal cards to the playfield
    for (int i = 7; i >= 1; i--) {
        for (int j = 7 - i; j < 7; j++) {
            Card c = stock.back();
            stock.pop_back();
            c.upturned = j == 7 - i;
            playfield.at(j).push_back(c);
        }
    }
}

void Game::run() {
    setup_game();

    bool exiting = false;
    Timer timer;
    SDL_Event e;

    while (!exiting) {
        mouse.update();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exiting = true;
            } if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {
                setup_game();
            } else {
                mouse.handle_input(e);
            }
        }

        float time = timer.elapsed();
        timer.reset();
        update(time);
        render();
    }
}

const Card& Game::get_card(CardSource src, std::pair<int, int> coord) {
    switch (src) {
        case CS_Pile:
            if (pile.empty()) {
                throw std::runtime_error("Pile is empty but get_card was called on it");
            }

            return pile.back();

        case CS_Playfield:
            return playfield.at(coord.first).at(coord.second);

        case CS_Aces:
            if (aces.at(coord.first).empty()) {
                throw std::runtime_error(std::format("Aces pile {} empty but get_card was called on it", coord.first));
            }

            return aces.at(coord.first).back();
        default:
            throw std::runtime_error("Invalid card source");
    }
}

std::vector<Card> Game::pop_cards(CardSource src, std::pair<int, int> coord) {
    std::vector<Card> res;
    switch (src) {
        case CS_Pile:
            if (pile.empty()) {
                throw std::runtime_error("Pile is empty but get_card was called on it");
            }

            res.push_back(pile.back());
            pile.pop_back();
            break;

        case CS_Playfield:
            for (int i = coord.second; i < (int)playfield.at(coord.first).size(); i++) {
                res.push_back(playfield.at(coord.first).at(i));
            }

            playfield.at(coord.first).erase(playfield.at(coord.first).begin() + coord.second, playfield.at(coord.first).end());

            if (!playfield.at(coord.first).empty()) {
                playfield.at(coord.first).back().upturned = true;
            }
            break;

        case CS_Aces:
            if (aces.at(coord.first).empty()) {
                throw std::runtime_error(std::format("Aces pile {} empty but get_card was called on it", coord.first));
            }

            res.push_back(aces.at(coord.first).back());
            aces.at(coord.first).pop_back();
    }

    return res;
}

bool Game::is_solved() {
    bool all_upturned = true;

    for (int i = 0; i < 7; i++) {
        for (auto c = playfield.at(i).begin(); c != playfield.at(i).end(); c++) {
            all_upturned &= c->upturned;
        }
    }

    return pile.size() == 0 && stock.size() == 0 && all_upturned;
}

void Game::run_ai() {
    if (!useAi) {
        throw std::runtime_error("runAi called but ai is not being used");
    }

    std::optional<SolitaireMove> move = ai->nextMove(playfield, pile, aces);

    if (!move) {
        return;
    }

    if (std::holds_alternative<CyclePile>(*move)) {
        deal_or_reset_stock();
    } else if (std::holds_alternative<MoveToStack>(*move)) {
        MoveToStack m = std::get<MoveToStack>(*move);
        const Card& selectedCard = get_card(m.source, m.fromCoord);

        if (selectedCard.value == King) {
            if (!playfield.at(m.toStackId).empty()) {
                throw std::runtime_error("Ai tried to move a king to a nonempty space");
            }
        } else if (playfield.at(m.toStackId).empty() || !selectedCard.can_be_placed_on(playfield.at(m.toStackId).back())) {
            throw std::runtime_error("Ai tried to place a card on a card that it cant go on");
        }

        std::vector<Card> cards = pop_cards(m.source, m.fromCoord);
        for (auto c = cards.begin(); c != cards.end(); c++) {
            playfield.at(m.toStackId).push_back(*c);
        }
    } else if (std::holds_alternative<MoveToAces>(*move)) {
        MoveToAces m = std::get<MoveToAces>(*move);
        const Card& selectedCard = get_card(m.source, m.fromCoord);

        if (selectedCard.value == Ace) {
            if (!aces.at(m.toAcesId).empty()) {
                throw std::runtime_error("Ai tried to put an ace on a non-empty ace space");
            }
        } else if (aces.at(m.toAcesId).empty()
                || static_cast<int>(selectedCard.value) != static_cast<int>(aces.at(m.toAcesId).back().value) + 1
                || selectedCard.suit != aces.at(m.toAcesId).back().suit)
        {
            throw std::runtime_error(std::format(
                "Ai tried to put a card in an ace space where it cant go. card value: {}, suit: {}",
                static_cast<int>(selectedCard.value),
                static_cast<int>(selectedCard.suit)
            ));
        }

        std::vector<Card> cards = pop_cards(m.source, m.fromCoord);
        if (cards.size() != 1) {
            throw std::runtime_error("Ai played an invalid move!");
        }

        aces.at(m.toAcesId).push_back(cards[0]);
    }
}

void Game::update(float dt) {
    if (useAi) {
        if (is_solved()) {
            return;
        }

        aiMoveTimer += dt;

        if (true) {
            aiMoveTimer -= AI_MOVE_TIME;
    
            run_ai();
        }
    }

    else {
        if (mouse.is_just_pressed(1)) {
            // Check if the stock was pressed
            auto mp = mouse.pos();
            if (is_hovering_card(mp, { STOCK_X, STOCK_PILE_Y }, 0)) {
                deal_or_reset_stock();
            }

            else if (!held) {
                // Check to see if the player picked up a new card
                std::optional<HeldCard> newHeld = get_hovered_card(0);
                held = newHeld;
            }
        }

        else if (mouse.is_just_released(1)) {
            if (held) {
                std::optional<HeldCard> hovered = get_hovered_card(5);

                // If we are hovering over a stack card, we should try to place the card on that stack
                if (hovered && hovered->stackCoord && hovered->stackCoord->second == (int)playfield.at(hovered->stackCoord->first).size() - 1) {
                    std::vector<Card>& stack = playfield.at(hovered->stackCoord->first);
                    // important that c isn't a reference bc we'll be modifying stack later and we don't want it to be invalidated
                    // c++ moment!
                    Card c = stack.back();

                    if (held->c.can_be_placed_on(c)) {
                        if (held->stackCoord) {
                            // If the held card is from a stack, we need to place all the cards that were below it too
                            std::vector<Card>& fromStack = playfield.at(held->stackCoord->first);

                            for (int j = held->stackCoord->second; j < (int)fromStack.size(); j++) {
                                stack.push_back(fromStack.at(j));
                            }

                            pop_held_cards();
                        } else {
                            // If it was from the pile, just place the card
                            stack.push_back(pile.back());
                            pop_held_cards();
                        }
                    }
                } else if (auto acesId = get_hovered_aces_id(5); acesId) {
                    bool canBePlaced = (held->c.value == Ace && aces.at(*acesId).empty())
                        || (!aces.at(*acesId).empty()
                                && static_cast<int>(aces.at(*acesId).back().value) == static_cast<int>(held->c.value) - 1
                                && aces.at(*acesId).back().suit == held->c.suit);

                    if (canBePlaced) {
                        aces.at(*acesId).push_back(held->c);
                        pop_held_cards();
                    }
                } else if (auto emptyId = get_hovered_empty_id(5); emptyId && held->c.value == King) {
                    if (!playfield.at(*emptyId).empty()) {
                        throw std::runtime_error("Stack not empty but it should be");
                    }

                    if (held->stackCoord) {
                        std::vector<Card>& fromStack = playfield.at(held->stackCoord->first);

                        for (int j = held->stackCoord->second; j < (int)fromStack.size(); j++) {
                            playfield.at(*emptyId).push_back(fromStack.at(j));
                        }
                    } else {
                        playfield.at(*emptyId).push_back(held->c);
                    }

                    pop_held_cards();
                }

                held = std::nullopt;
            }
        }
    }
}

void Game::pop_held_cards() {
    if (held->stackCoord) {
        if (playfield.at(held->stackCoord->first).empty()) {
            throw std::runtime_error("Stack should not be empty rn");
        }

        std::vector<Card>& stack = playfield.at(held->stackCoord->first);
        stack.erase(stack.begin() + held->stackCoord->second, stack.end());

        if (!stack.empty() && !stack.back().upturned) {
            stack.back().upturned = true;
        }
    } else {
        if (pile.empty()) {
            throw std::runtime_error("Stack should not be empty rn");
        }
        pile.pop_back();
    }
}

std::optional<int> Game::get_hovered_aces_id(int tolerance) {
    auto mp = mouse.pos();

    for (int i = 0; i < 4; i++) {
        int x = ACES_X + i * ACES_DX;
        int y = STOCK_PILE_Y;

        if (is_hovering_card(mp, { x, y }, tolerance)) {
            return i;
        }
    }

    return std::nullopt;
}

std::optional<int> Game::get_hovered_empty_id(int tolerance) {
    auto mp = mouse.pos();

    for (int i = 0; i < 7; i++) {
        if (!playfield.at(i).empty()) {
            continue;
        }

        int x = PLAYFIELD_START_X + i * PLAYFIELD_CARD_DX;
        int y = PLAYFIELD_START_Y;

        if (is_hovering_card(mp, { x, y }, tolerance)) {
            return i;
        }
    }

    return std::nullopt;
}

std::optional<HeldCard> Game::get_hovered_card(int tolerance) {
    auto mp = mouse.pos();

    // Check if we are hovering over the pile card
    if (!pile.empty()) {
        int pile_x = PILE_X + (std::min((int)pile.size(), 3) - 1) * PILE_DX;
        int pile_y = STOCK_PILE_Y;

        if (is_hovering_card(mp, { pile_x, pile_y }, tolerance)) {
            return HeldCard(pile.back(), { mp.first - pile_x, mp.second - pile_y });
        }
    }

    // Check the stacks
    for (int i = 0; i < 7; i++) {
        std::vector<Card>& stack = playfield.at(i);
        if (stack.empty()) {
            continue;
        }

        int height = stack_height(i);
        int x = PLAYFIELD_START_X + i * PLAYFIELD_CARD_DX;
        int y = PLAYFIELD_START_Y + height;

        for (int j = stack.size() - 1; j >= 0; j--) {
            Card c = stack.at(j);
            if (!c.upturned) {
                break;
            }

            if (is_hovering_card(mp, { x, y }, tolerance)) {
                return HeldCard(c, { mp.first - x, mp.second - y }, { i, j });
            }

            y -= PLAYFIELD_UP_CARD_DY;
        }
    }

    return std::nullopt;
}

// The distance between the top of the stack and the y position of the last card on the stack.
// i.e., the height of the "covered card" section
int Game::stack_height(int i) {
    int height = 0;
    std::vector<Card>& stack = playfield.at(i);

    for (unsigned int j = 0; j < stack.size() - 1; j++) {
        Card& c = stack.at(j);
        if (c.upturned) {
            height += PLAYFIELD_UP_CARD_DY;
        } else {
            height += PLAYFIELD_DOWN_CARD_DY;
        }
    }

    return height;
}

void Game::deal_or_reset_stock() {
    if (stock.empty()) {
        // Move cards from the pile to the stock
        while (!pile.empty()) {
            Card c = pile.back();
            pile.pop_back();
            stock.push_back(c);
        }
    } else {
        // Deal up to 3 cards
        for (int i = 0; i < cardDraw; i++) {
            if (stock.empty()) {
                break;
            }

            Card c = stock.back();
            stock.pop_back();
            pile.push_back(c);
        }
    }
}

void Game::render() {
    renderer.set_draw_colour(0x34, 0xC9, 0x70, 0xFF);
    renderer.clear();

    // Render the playfield
    for (int i = 0; i < 7; i++) {
        int x = PLAYFIELD_START_X + i * PLAYFIELD_CARD_DX;
        int y = PLAYFIELD_START_Y;
        std::vector<Card>& stack = playfield.at(i);

        for (int j = 0; j < (int)stack.size(); j++) {
            Card& c = stack.at(j);

            if (held && held->stackCoord && held->stackCoord->first == i && held->stackCoord->second == j) {
                break;
            }

            if (c.upturned) {
                render_card(c, x, y);
                y += PLAYFIELD_UP_CARD_DY;
            } else {
                render_card_back(x, y);
                y += PLAYFIELD_DOWN_CARD_DY;
            }
        }
    }

    // Render the stock and pile
    if (!stock.empty()) {
        render_card_back(STOCK_X, STOCK_PILE_Y);
    } else {
        render_card_outline(STOCK_X, STOCK_PILE_Y);
    }

    // Render the pile
    if (!pile.empty()) {
        if (held && !held->stackCoord) {
            if (pile.size() > 1) {
                int x = PILE_X;
                for (int i = std::min((int)pile.size() - 1, 2); i > 0; i--) {
                    render_card(pile.at(pile.size() - i - 1), x, STOCK_PILE_Y);
                    x += PILE_DX;
                }
            } else {
                render_card_outline(PILE_X, STOCK_PILE_Y);
            }
        } else {
            int x = PILE_X;
            for (int i = std::min((int)pile.size(), 3); i > 0; i--) {
                render_card(pile.at(pile.size() - i), x, STOCK_PILE_Y);
                x += PILE_DX;
            }
        }
    }

    // Render the ace stacks
    for (int i = 0; i < 4; i++) {
        if (aces.at(i).empty()) {
            render_card_outline(ACES_X + i * ACES_DX, STOCK_PILE_Y);
        } else {
            render_card(aces.at(i).back(), ACES_X + i * ACES_DX, STOCK_PILE_Y);
        }
    }

    // Render the held card
    if (held) {
        auto mp = mouse.pos();
        int x = mp.first - held->mouseOffset.first;
        int y = mp.second - held->mouseOffset.second;

        if (held->stackCoord) {
            std::vector<Card>& stack = playfield.at(held->stackCoord->first);

            for (int j = held->stackCoord->second; j < (int)stack.size(); j++) {
                render_card(stack.at(j), x, y + (j - held->stackCoord->second) * PLAYFIELD_UP_CARD_DY);
            }
        } else {
            // Card is from the pile
            if (pile.empty()) {
                throw std::runtime_error("Pile is empty but held card is from the pile?");
            }

            render_card(pile.back(), x, y);
        }
    }

    renderer.present();
}

SDL_Rect get_rect_for_tile(const std::pair<int, int>& coord) {
    SDL_Rect res;
    res.x = coord.first * CARD_TILE_WIDTH + CARD_TILE_OFFSET_X;
    res.y = coord.second * CARD_TILE_HEIGHT + CARD_TILE_OFFSET_Y;
    res.w = CARD_SPRITE_WIDTH;
    res.h = CARD_SPRITE_HEIGHT;

    return res;
}

std::pair<int, int> card_tilesheet_coord(const Card& card) {
    return { static_cast<int>(card.value), static_cast<int>(card.suit) };
}

SDL_Rect get_rect_for_card(const Card& card) {
    return get_rect_for_tile(card_tilesheet_coord(card));
}

void Game::render_card(const Card& card, int x, int y) {
    SDL_Rect srcRect = get_rect_for_card(card);
    SDL_Rect dstRect;
    dstRect.x = x;
    dstRect.y = y;
    dstRect.w = CARD_SPRITE_WIDTH * CARD_UPSCALE;
    dstRect.h = CARD_SPRITE_HEIGHT * CARD_UPSCALE;
    renderer.draw_texture(cardTexture, std::make_optional(srcRect), dstRect);
}

void Game::render_card_back(int x, int y) {
    SDL_Rect srcRect = get_rect_for_tile({ 13, 1 });
    SDL_Rect dstRect;
    dstRect.x = x;
    dstRect.y = y;
    dstRect.w = CARD_SPRITE_WIDTH * CARD_UPSCALE;
    dstRect.h = CARD_SPRITE_HEIGHT * CARD_UPSCALE;
    renderer.draw_texture(cardTexture, std::make_optional(srcRect), dstRect);
}

void Game::render_card_outline(int x, int y) {
    SDL_Rect dstRect;
    dstRect.x = x;
    dstRect.y = y;
    dstRect.w = CARD_SPRITE_WIDTH * CARD_UPSCALE;
    dstRect.h = CARD_SPRITE_HEIGHT * CARD_UPSCALE;
    renderer.draw_texture(cardOutline, std::nullopt, dstRect);
}
