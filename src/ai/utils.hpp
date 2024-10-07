#include <vector>
#include "ai.hpp"

std::vector<SolitaireMove> possible_moves_for_card(
    const Card& c,
    bool isSingle /* for you this is always true */,
    CardSource src,
    std::pair<int, int> srcCoord,
    const std::array<std::vector<Card>, 7>& playfield,
    const std::array<std::vector<Card>, 4>& aces
);

std::vector<SolitaireMove> possible_moves(
    const std::array<std::vector<Card>, 7>& playfield,
    const std::vector<Card>& pile,
    const std::array<std::vector<Card>, 4>& aces
);
