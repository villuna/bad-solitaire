#include "benchmark.hpp"
#include "../game.hpp"
#include "../utils.hpp"
#include "ai.hpp"
#include <print>

const int MAX_TURNS = 400;
const int GAMES = 10000;

void benchmark(int draw, std::unique_ptr<SolitaireAI> ai) {
    Game g(draw, std::move(ai));
    int games = GAMES;
    int wins = 0;
    std::vector<int> turnCounts;
    std::vector<float> times;

    for (int i = 0; i < games; i++) {
        g.setup_game();
        Timer t;

        for (int turn = 0; turn < MAX_TURNS; turn++) {
            g.run_ai();

            if (g.is_solved()) {
                float time = t.elapsed();
                times.push_back(time);
                turnCounts.push_back(turn + 1);
                wins++;
                break;
            }
        }
    }

    std::println("ai won {} out of {} games. (wr: {}%)", wins, games, 100 * static_cast<float>(wins) / static_cast<float>(games));

    // For saving results to plot
    // I tried this but found the results weren't very interesting
    // curse you, central limit theorem!
    //
    // std::ofstream ofs { "output.txt" };

    // for (int i = 0; i < (int)turnCounts.size(); i++) {
    //     if (i != 0) {
    //         ofs << ",";
    //     }
    //     ofs << turnCounts[i];
    // }
    // ofs << std::endl;

    // for (int i = 0; i < (int)times.size(); i++) {
    //     if (i != 0) {
    //         ofs << ",";
    //     }
    //     ofs << times[i];
    // }
    // ofs << std::endl;

    if (wins > 0) {
        float totalTurns = 0, totalTime = 0;

        for (auto i = turnCounts.begin(); i != turnCounts.end(); i++) {
            totalTurns += static_cast<float>(*i);
        }

        for (auto i = times.begin(); i != times.end(); i++) {
            totalTime += *i;
        }

        std::println("Average turn count: {}.", totalTurns / static_cast<float>(turnCounts.size()));
        std::println("Average time: {}s", totalTime / static_cast<float>(times.size()));
    }
}
