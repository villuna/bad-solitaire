#include "utils.hpp"

std::vector<SolitaireMove> possible_moves_for_card(
    const Card& c,
    bool isSingle /* for you this is always true */,
    CardSource src,
    std::pair<int, int> srcCoord,
    const std::array<std::vector<Card>, 7>& playfield,
    const std::array<std::vector<Card>, 4>& aces
) {
    std::vector<SolitaireMove> res;

    // Don't move kings that are already on an empty space on the board
    // it just doesn't do anything
    if (c.value == King && src == CS_Playfield && srcCoord.second == 0) {
        return res;
    }

    for (int i = 0; i < 7; i++) {
        if (src == CS_Playfield && srcCoord.first == i) {
            continue;
        }

        if ((c.value == King && playfield.at(i).empty())
            || (!playfield.at(i).empty() && c.can_be_placed_on(playfield.at(i).back())))
        {
            res.push_back(MoveToStack { .source = src, .fromCoord = srcCoord, .toStackId = i });
        }
    }

    if (isSingle && src != CS_Aces) {
        for (int i = 0; i < 4; i++) {
            if ((aces.at(i).empty() && c.value == Ace)
                || (!aces.at(i).empty() && static_cast<int>(c.value) == static_cast<int>(aces.at(i).back().value) + 1 && c.suit == aces.at(i).back().suit))
            {
                res.push_back(MoveToAces { .source = src, .fromCoord = srcCoord, .toAcesId = i });
            }
        }
    }

    return res;
}

std::vector<SolitaireMove> possible_moves(
    const std::array<std::vector<Card>, 7>& playfield,
    const std::vector<Card>& pile,
    const std::array<std::vector<Card>, 4>& aces
) {
    std::vector<SolitaireMove> moves { CyclePile {} };

    if (!pile.empty()) {
        auto pile_moves = possible_moves_for_card(pile.back(), true, CS_Pile, { }, playfield, aces);
        for (auto m = pile_moves.begin(); m != pile_moves.end(); m++) {
            moves.push_back(*m);
        }
    }

    for (int i = 0; i < 4; i++) {
        if (!aces.at(i).empty()) {
            auto ace_moves = possible_moves_for_card(aces.at(i).back(), true, CS_Aces, { i, 0 }, playfield, aces);
            for (auto m = ace_moves.begin(); m != ace_moves.end(); m++) {
                moves.push_back(*m);
            }
        }
    }

    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < (int)playfield.at(i).size(); j++) {
            const Card& c = playfield.at(i).at(j);

            if (c.upturned) {
                auto card_moves = possible_moves_for_card(
                    c,
                    j == (int)playfield.at(i).size() - 1,
                    CS_Playfield,
                    { i, j },
                    playfield,
                    aces
                );

                for (auto m = card_moves.begin(); m != card_moves.end(); m++) {
                    moves.push_back(*m);
                }
            }
        }
    }

    return moves;
}
