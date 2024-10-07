#include "ai.hpp"
#include <optional>

// My attempt at making a good solitaire bot that tries to predict good moves
class Kiki : SolitaireAI {
public:
    Kiki();

    std::optional<SolitaireMove> nextMove(
        const std::array<std::vector<Card>, 7>& playfield,
        const std::vector<Card>& pile,
        const std::array<std::vector<Card>, 4>& aces
    ) override;
};
