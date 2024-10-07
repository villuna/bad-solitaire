#pragma once

enum Value {
    Ace,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Ten,
    Jack,
    Queen,
    King,
};

enum Suit {
    Hearts,
    Diamonds,
    Clubs,
    Spades,
};

struct Card {
    Value value;
    Suit suit;
    bool upturned;

    Card() {}
    Card(Value value, Suit suit) : value(value), suit(suit), upturned(true) {}
    Card(Value value, Suit suit, bool upturned) : value(value), suit(suit), upturned(upturned) {}

    bool can_be_placed_on(const Card& other) const;
};
