#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>
#include <numeric>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

enum class GuessFeedback {
    black = 0,
    yellow = 1,
    green = 2,

    not_present = 0,
    present_but_wrong_spot = 1,
    present_and_correct_spot = 2,
};

using FeedbackArray = std::array<GuessFeedback, 5>;

class Wordle {
public:
    friend class Filter;

    Wordle(const std::string_view& word) noexcept
    {
        std::transform(word.begin(), word.end(), std::back_inserter(word_), [](const char c)->char { return c - 'a'; });
    }

    int count(char c) const {
        return counts_[c - 'a'];
    }

    const char& operator[](std::size_t index) const {
        return word_[index] + 'a';
    }

    friend std::ostream& operator<<(std::ostream& os, const Wordle& wordle);

private:
    std::string word_{};
    std::array<char, 26> counts_{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};

std::ostream& operator<<(std::ostream& os, const Wordle& wordle) {
    for (const auto& c : wordle.word_) {
        os << static_cast<char>(c + 'a');
    }
    return os;
}

struct Filter {
public:
    // tags used to default create filters
    struct accept_all{};
    struct reject_all{};

    // template <typename InputIt>
    // Filter(InputIt first, InputIt last) {
    //     auto it = first;
    //     while (it != last) {
    //         _add_wordle(*it++);
    //     }
    // }

    Filter(const accept_all&) noexcept
        : possible_chars_{{
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}}}
        , min_chars_{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
        , max_chars_{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}
    { }

    Filter(const Wordle& wordle, const FeedbackArray& feedback)
        : Filter(accept_all{})
    {
        for (int i = 0; i < wordle.word_.size(); ++i) {
            const char& c = wordle.word_[i];
            const auto& f = feedback[i];

            switch (f) {
                case GuessFeedback::not_present:
                    max_chars_[c] = 0;
                    break;
                case GuessFeedback::present_but_wrong_spot:
                    ++min_chars_[c];
                    break;
                case GuessFeedback::present_and_correct_spot:
                    ++min_chars_[c];
                    break;
            }
        }

        int confirmed_chars = std::accumulate(
            min_each.begin(), min_each.end(), 0,
            [](int count, const std::pair<char, int>& val) { return count + val.second; }
        );

        for (auto& [k, v] : min_each) {
            if (max_each.count(k)) {
                max_each[k] = v;
            }
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Filter& filter);

private:
    void _add_wordle(const Wordle& wordle) {
        for (int i = 0; i < 5; ++i) {
            const auto& c = wordle.word_[i];
            possible_chars_[i][c] = true;
        }
        for (int i = 0; i < 26; ++i) {
            min_chars_[i] = std::min(static_cast<const char>(min_chars_[i]), wordle.counts_[i]);
            max_chars_[i] = std::max(static_cast<const char>(min_chars_[i]), wordle.counts_[i]);
        }
    }

private:
    std::array<std::array<bool, 26>, 5> possible_chars_{{
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    }};
    std::array<int, 26> min_chars_{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5};
    std::array<int, 26> max_chars_{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};

std::ostream& operator<<(std::ostream& os, const Filter& filter) {
    auto format_flags = os.flags();
    os << std::right << std::setfill(' ');

    os << "   ";
    for (char c = 'a'; c <= 'z'; ++c) {
        os << std::setw(2) << c;
    }
    os << '\n';

    for (int i = 0; i < filter.possible_chars_.size(); ++i) {
        const auto& row = filter.possible_chars_[i];
        os << std::setw(3) << i;
        for (const auto& cell : row) {
            os << std::setw(2) << (cell ? 'o' : '.');
        }
        os << '\n';
    }

    os << std::setw(3) << "min";
    for (const auto& cell : filter.min_chars_) {
        os << std::setw(2) << static_cast<int>(cell);
    }
    os << '\n';

    os << std::setw(3) << "max";
    for (const auto& cell : filter.max_chars_) {
        os << std::setw(2) << static_cast<int>(cell);
    }
    os << '\n';

    os.flags(format_flags);
    return os;
}

// class SearchSpace {
// public:
//     bool contains(const std::string_view& wordle) const {
//         return std::binary_search(wordles_.begin(), wordles_.end(), std::string(wordle));
//     }

//     std::size_t size_if_reduced(const std::string_view& guess, const std::array<GuessInfo, 5>& info) const {

//     }

//     void reduce(const std::string_view& guess, const std::array<GuessInfo, 5>& info) {
//         std::map<int, std::set<char>> possible_chars;
//         std::map<char, int> min_each;
//         std::map<char, int> max_each;

//         for (int i = 0; i < guess.size(); ++i) {
//             char c = guess[i];
//             auto c_info = info[i];

//             switch (c_info) {
//                 case GuessInfo::not_present:
//                     max_each[c] = 0;
//                     possible_chars[i].erase(c);
//                     break;
//                 case GuessInfo::present_but_wrong_spot:
//                     ++min_each[c];
//                     possible_chars[i].erase(c);
//                     break;
//                 case GuessInfo::present_and_correct_spot:
//                     ++min_each[c];
//                     possible_chars[i] = std::set<char>{c};
//                     break;
//             }
//         }

//         int confirmed_chars = std::accumulate(
//             min_each.begin(), min_each.end(), 0,
//             [](int count, const std::pair<char, int>& val) { return count + val.second; }
//         );

//         for (auto& [k, v] : min_each) {
//             if (max_each.count(k)) {
//                 max_each[k] = v;
//             }
//         }

//         // now, remove words that do not match
//         const auto& cmin_each = min_each;
//         const auto& cmax_each = max_each;
//         const auto& cpossible_chars = possible_chars;
//         wordles_.erase(std::remove_if(wordles_.begin(), wordles_.end(), [&](const std::string& wordle)->bool {
//             for (const auto& [letter, min_count] : cmin_each) {

//             }
//         }), wordles_.end());
//     }

//     void add_wordle(const std::string_view& wordle) {
//         wordles_.emplace_back(wordle);
//         std::sort(wordles_.begin(), wordles_.end());
//     }

//     template <typename InputIt>
//     void add_wordles(InputIt first, InputIt last) {
//         wordles_.resize(wordles_.size() + std::distance(first, last));
//         while (first != last) {
//             wordles.emplace_back(*first++);
//         }
//         std::sort(wordles_.begin(), wordles_.end());
//     }

//     std::size_t size() const {
//         return wordles_.size();
//     }

// private:
//     std::vector<std::string> wordles_{};
// };

int main(int argc, char** argv) {
    
}
