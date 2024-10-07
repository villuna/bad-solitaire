#include "dennis.hpp"
#include "ai.hpp"
#include "utils.hpp"
#include <print>
#include <random>
#include <algorithm>
#include <stdexcept>
#include <variant>

Dennis::Dennis() :
    rand(std::mt19937 { std::random_device{}() })
{}

const int VAL_FROM_ACES = 0;
const int VAL_MOVE_NOT_UNCOVERING = 1;
const int VAL_TO_ACES = 2;
const int VAL_FROM_PILE = 3;
const int VAL_MOVE_UNCOVERING = 4;

int move_value(
    const SolitaireMove& move,
    const std::array<std::vector<Card>, 7>& playfield,
    const std::vector<Card>& pile,
    const std::array<std::vector<Card>, 4>& aces
) {
    // The basic value of moves in this strategy is this (worst to best)
    //
    // 0 - move down from the aces
    // 1 - move playfield card, not uncovering new card
    // 2 - move down from the pile
    // 3 - move playfield card, uncovering new card
    // 4 - move to aces
    //
    // We don't consider cycling the pile here. Since it can always be done, if it had a
    // value greater than 0 it would negate all moves below it. Instead, we give every move the
    // chance to cycle the pile, and that chance changes based on the value of the best possible move.
    if (std::holds_alternative<MoveToStack>(move)) {
        MoveToStack m = std::get<MoveToStack>(move);
        if (m.source == CS_Aces) {
            return VAL_FROM_ACES;
        } else if (m.source == CS_Playfield) {
            const std::vector<Card>& fromStack = playfield.at(m.fromCoord.first);
            
            if (m.fromCoord.second == 0) {
                // TODO: maybe consider checking if there is a king available anywhere
                return VAL_MOVE_NOT_UNCOVERING; 
            } else if (fromStack.at(m.fromCoord.second - 1).upturned) {
                return VAL_MOVE_NOT_UNCOVERING;
            } else {
                return VAL_MOVE_UNCOVERING;
            }
        } else {
            return VAL_FROM_PILE;
        }
    } else if (std::holds_alternative<MoveToAces>(move)) {
        return VAL_TO_ACES;
    } else {
        throw std::runtime_error("Move cannot be evaluated."); 
    }
}

bool compare_moves(
    const SolitaireMove& a,
    const SolitaireMove& b,
    const std::array<std::vector<Card>, 7>& playfield,
    const std::vector<Card>& pile,
    const std::array<std::vector<Card>, 4>& aces
) {
    int valuea = move_value(a, playfield, pile, aces);
    int valueb = move_value(b, playfield, pile, aces);
    return valuea < valueb;
}

int cycle_pile_percentage(
    const SolitaireMove& move,
    const std::array<std::vector<Card>, 7>& playfield,
    const std::vector<Card>& pile,
    const std::array<std::vector<Card>, 4>& aces
) {
    int val = move_value(move, playfield, pile, aces);

    switch (val) {
        case VAL_FROM_ACES:
        case VAL_MOVE_NOT_UNCOVERING:
            return 90;
        case VAL_FROM_PILE:
        case VAL_MOVE_UNCOVERING:
        case VAL_TO_ACES:
            return 0;
        default:
            return 100;
    }
}

std::optional<SolitaireMove> Dennis::nextMove(
    const std::array<std::vector<Card>, 7>& playfield,
    const std::vector<Card>& pile,
    const std::array<std::vector<Card>, 4>& aces
) {
    std::vector<SolitaireMove> moves = possible_moves(playfield, pile, aces);

    if (!moves.empty() && std::holds_alternative<CyclePile>(moves[0])) {
        moves.erase(moves.begin());
    }

    std::sort(
        moves.begin(),
        moves.end(),
        [&, playfield, pile, aces](const SolitaireMove& a, const SolitaireMove& b) {
            return compare_moves(a, b, playfield, pile, aces);
        }
    );

    if (moves.empty()) {
        return CyclePile {};
    } else {
        const SolitaireMove& move = moves.back();
        int cycleProb = cycle_pile_percentage(move, playfield, pile, aces);

        if (int r = (unsigned int) rand() % 100; r < cycleProb) {
            return CyclePile {};
        } else {
            return move;
        }
    }
}
