#pragma once

#include "ai.hpp"
#include <optional>
#include <random>
#include <vector>

// A solitaire bot that knows nothing except the rules.
// Will make a random move until it solves the game.
// To make sure that it doesn't flounder around *too* much, I might at least stop it once the
// game is trivially solvable
class Pippin : public SolitaireAI {
public:
    Pippin();

    std::optional<SolitaireMove> nextMove(
        const std::array<std::vector<Card>, 7>& playfield,
        const std::vector<Card>& pile,
        const std::array<std::vector<Card>, 4>& aces
    ) override;

private:
    std::mt19937 rand;
};
