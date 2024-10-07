#pragma once

#include "ai.hpp"
#include <optional>
#include <random>

// A naive solitaire bot that just makes whatever (non-frivolous) moves it can
// Tries to reveal cards on the board. Failing that, tries to place cards from the pile.
// If there are no cards from the pile that it can place, tries again and makes random shifting
// moves up until a certain point when it will give up.
//
// My attempt to emulate a sort of "naive human" strategy without any kind of fancy lookahead.
class Dennis : public SolitaireAI {
public:
    Dennis();

    std::optional<SolitaireMove> nextMove(
        const std::array<std::vector<Card>, 7>& playfield,
        const std::vector<Card>& pile,
        const std::array<std::vector<Card>, 4>& aces
    ) override;

private:
    std::mt19937 rand;
};
