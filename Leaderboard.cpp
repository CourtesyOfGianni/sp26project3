#include "Leaderboard.hpp"
#include <algorithm>
#include <chrono>

/**
 * @brief Constructor for RankingResult with top players, cutoffs, and elapsed time.
 *
 * @param top Vector of top-ranked Player objects, in sorted order.
 * @param cutoffs Map of player count thresholds to minimum level cutoffs.
 *   NOTE: This is only ever non-empty for Online::rankIncoming().
 *         This parameter & the corresponding member should be empty
 *         for all Offline algorithms.
 * @param elapsed Time taken to calculate the ranking, in seconds.
 */
RankingResult::RankingResult(const std::vector<Player>& top, const std::unordered_map<size_t, size_t>& cutoffs, double elapsed)
    : top_ { top }
    , cutoffs_ { cutoffs }
    , elapsed_ { elapsed }
{
}



namespace Offline {

    RankingResult heapRank(std::vector<Player>& players) {
    int k = players.size() / 10; //top 10% of players rounded down

    auto start = std::chrono::high_resolution_clock::now();

    std::make_heap(players.begin(), players.end()); //turn the vector into a max heap

    for (int i = 0; i < k; ++i) {
        std::pop_heap(players.begin(), players.end() - i); //move the max element to the end of the vector
    }
    auto top_players_start = players.end() - k; //get the top k players
    std::reverse(top_players_start, players.end()); //reverse the order to get the top players in sorted order

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();
   
    return RankingResult(std::vector<Player>(top_players_start, players.end()), {}, elapsed);
}

int partition(std::vector<Player>& players, int left, int right) {
    Player pivot = players[right];
    int i = left - 1;
    for (int j = left; j < right; ++j) {
        if (players[j] < pivot) {
            ++i;
            std::swap(players[i], players[j]);
        }
    }
    std::swap(players[i + 1], players[right]);
    return i + 1;
}

void quickSelectHelper(std::vector<Player>& players, int left, int right, int boundary) {
    if (left >= right) return;
    
    int pivot_index = partition(players, left, right);
    
    if (pivot_index == boundary) {
        std::sort(players.begin() + pivot_index, players.begin() + right + 1);
    } else if (pivot_index < boundary) {
        quickSelectHelper(players, pivot_index + 1, right, boundary);
    } else {
        quickSelectHelper(players, left, pivot_index - 1, boundary);
    }
}

RankingResult quickSelectRank(std::vector<Player>& players) {
    int k = players.size() / 10;
    int boundary = players.size() - k;

    auto start = std::chrono::high_resolution_clock::now();

    quickSelectHelper(players, 0, players.size() - 1, boundary);

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

    return RankingResult(std::vector<Player>(players.begin() + boundary, players.end()), {}, elapsed);
}

} // namespace Offline

namespace Online {

    void replaceMin(PlayerIt first, PlayerIt last, Player& target) {
    *first = target;
    
    size_t size = last - first;
    size_t i = 0;
    
    while (true) {
        size_t left = 2*i + 1;
        size_t right = 2*i + 2;
        size_t smallest = i;
        
        if (left < size && *(first + left) < *(first + smallest))
            smallest = left;
        if (right < size && *(first + right) < *(first + smallest))
            smallest = right;
        if (smallest == i) break;
        
        std::swap(*(first + i), *(first + smallest));
        i = smallest;
    }
}

RankingResult rankIncoming(PlayerStream& stream, const size_t& reporting_interval) {
    std::vector<Player> min_heap;
    std::unordered_map<size_t, size_t> cutoffs;
    size_t total_players = 0;
    double elapsed = 0;

    auto cmp = [](const Player& a, const Player& b) { return a.level_ > b.level_; };

    while (stream.remaining() > 0) {
        Player next = stream.nextPlayer();
        ++total_players;

        auto start = std::chrono::high_resolution_clock::now();

        if (min_heap.size() < reporting_interval) {
            min_heap.push_back(next);
            if (min_heap.size() == reporting_interval) {
                std::make_heap(min_heap.begin(), min_heap.end(), cmp);
            }
        } else if (next > min_heap.front()) {
            replaceMin(min_heap.begin(), min_heap.end(), next);
        }

        if (total_players % reporting_interval == 0) {
            cutoffs[total_players] = min_heap.front().level_;
        }

        auto end = std::chrono::high_resolution_clock::now();
        elapsed += std::chrono::duration<double, std::milli>(end - start).count();
    }

    if (total_players % reporting_interval != 0) {
        cutoffs[total_players] = min_heap.front().level_;
    }

    std::sort(min_heap.begin(), min_heap.end());
    return RankingResult(min_heap, cutoffs, elapsed);
}


} // namespace Online