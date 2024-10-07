#pragma once

// An abstract class that represents an ai that plays solitaire
#include "src/cards.hpp"
#include <array>
#include <optional>
#include <variant>
#include <vector>

enum CardSource {
    CS_Pile,
    CS_Playfield,
    CS_Aces,
};

struct MoveToStack {
    CardSource source;
    std::pair<int, int> fromCoord;
    int toStackId;
};

struct MoveToAces {
    CardSource source;
    std::pair<int, int> fromCoord;
    int toAcesId;
};

struct CyclePile {};

typedef std::variant<MoveToStack, MoveToAces, CyclePile> SolitaireMove;

class SolitaireAI {
public:
    // Returns what move it thinks it should make given the state of the board
    virtual std::optional<SolitaireMove> nextMove(
        const std::array<std::vector<Card>, 7>& playfield,
        const std::vector<Card>& pile,
        const std::array<std::vector<Card>, 4>& aces
    ) = 0;

    virtual ~SolitaireAI() {}
};
