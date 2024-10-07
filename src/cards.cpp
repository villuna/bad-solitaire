#include "cards.hpp"

bool is_red(Suit s) {
    switch (s) {
        case Hearts:
        case Diamonds:
            return true;
        default:
            return false;
    }
}

bool Card::can_be_placed_on(const Card& other) const {
    // Cards must be different suits and this one must be 1 less than the other in value
    return is_red(this->suit) != is_red(other.suit)
        && static_cast<int>(this->value) == static_cast<int>(other.value) - 1;
}
