#include "Leaderboard.hpp"
#include "PlayerStream.hpp"

int main() {
    std::vector<Player> players = {
        Player("A", 5), Player("B", 3), Player("C", 8),
        Player("D", 1), Player("E", 9), Player("F", 2),
        Player("G", 7), Player("H", 4), Player("I", 6),
        Player("J", 10), Player("K", 15), Player("L", 12),
        Player("M", 11), Player("N", 14), Player("O", 13),
        Player("P", 16), Player("Q", 20), Player("R", 17),
        Player("S", 18), Player("T", 19)
    };
    auto result = Offline::heapRank(players);
    for (auto& p : result.top_) {
        std::cout << p.name_ << " " << p.level_ << std::endl;
    }
}