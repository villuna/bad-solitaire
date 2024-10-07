#include <SDL.h>
#include <SDL_image.h>
#include <cstring>
#include <iostream>
#include <print>
#include "game.hpp"
#include "ai/benchmark.hpp"
#include "ai/pippin.hpp"
#include "src/ai/ai.hpp"
#include "src/ai/dennis.hpp"

int main(int argc, char **argv) {
    if (argc == 3 && !std::strcmp(argv[1], "benchmark")) {
        std::unique_ptr<SolitaireAI> ai;

        if (!std::strcmp(argv[2], "dennis")) {
            ai = std::make_unique<Dennis>();
        } else if (!std::strcmp(argv[2], "pippin")) {
            ai = std::make_unique<Pippin>();
        } else {
            std::cerr << "Not a valid ai name: \"" << argv[2] << "\"" << std::endl;
            return 1;
        }

        benchmark(3, std::move(ai));
    } else if (argc == 1) {
        Game game(3);
        game.run();
    } else {
        std::println("Usage: bs");
        std::println("       bs benchmark [AI_NAME]");
        std::println("\nthe two ais to choose from right now are 'dennis' and 'pippin'.");
        return 1;
    }

    return 0;
}
