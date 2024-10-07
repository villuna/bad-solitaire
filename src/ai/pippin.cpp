#include "pippin.hpp"
#include "ai.hpp"
#include <random>
#include <print>
#include <variant>

#include "utils.hpp"

Pippin::Pippin():
    rand(std::mt19937 { std::random_device{}() })
{}

std::optional<SolitaireMove> Pippin::nextMove(
    const std::array<std::vector<Card>, 7>& playfield,
    const std::vector<Card>& pile,
    const std::array<std::vector<Card>, 4>& aces
) {
    std::vector<SolitaireMove> moves = possible_moves(playfield, pile, aces);

    for (int i = moves.size() - 1; i >= 0; i--) {
        auto m = moves[i];

        if (std::holds_alternative<MoveToStack>(m)) {
            auto mts = std::get<MoveToStack>(m); 
            if (mts.source == CS_Aces) {
                moves.erase(moves.begin() + i);
            }
        }
    }

    if (moves.empty()) {
        return std::nullopt;
    } else {
        return moves.at(rand() % moves.size());
    }
}
