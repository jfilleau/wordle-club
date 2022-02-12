#ifndef WORDLE_BAG_H
#define WORDLE_BAG_H

#include <unordered_map>

namespace wordle {

std::unordered_map<char, std::pair<int, int>> make_max_bag(int max_letters) {
    std::unordered_map<char, std::pair<int, int>> bag;

    bag.reserve(all_letters.size());
    for (const auto& c : all_letters) {
        bag[c] = std::make_pair(0, max_letters);
    }

    return bag;
}

template <typename InputIt>
int_fast32_t get_max_from_pairs(InputIt first, InputIt last) {
    int max = 0;
    while (first != last) {
        const auto& [letter, count] = first;
        const auto& [min_count, max_count] = count;
        max = std::max(max, max_count);
    }

    return max;
}

class Bag {
public:
    Bag(int available_selections = 5) noexcept
        : _global_max(available_selections)
        , _letters(make_max_bag(available_selections))
    { }

    template <typename InputIt>
    Bag(InputIt first, InputIt last) noexcept
        : _global_max(get_max_from_pairs(first, last))
        , _letters(first, last)
    { }

    void set_min(char c, int minimum) {
        _letters[c].first = minimum;
        _normalize();
    }

    void set_max(char c, int maximum) {
        _letters[c].second = maximum;
    }

    bool may_satisfy(const std::string& candidate_word) const {
        std::unordered_map<char, int> letter_counts;

        for (const auto& c : candidate_word) {
            letter_counts[c]++;
        }

        for (const auto& [c, v] : letter_counts) {
            if (v < _letters.at(c).first || v > _letters.at(c).second) {
                return false;
            }
        }

        return true;
    }

private:
    const int _global_max;
    std::unordered_map<char, std::pair<int, int>> _letters;

    void _normalize() {
        int reserved_letters = std::accumulate(
            _letters.begin(), _letters.end(), 0, [](const auto& accum, const auto& count) {
                return accum + count.first;
            }
        );

        int new_max = _global_max - reserved_letters;
        for (auto& [letter, count_range] : _letters) {
            auto& [count_min, count_max] = count_range;
            count_max = new_max;
        }
    }
};

} // namespace wordle

#endif // define