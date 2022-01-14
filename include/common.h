#ifndef WORDLE_CLUB_COMMON_H
#define WORDLE_CLUB_COMMON_H

#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <ostream>

namespace wordle_club {

enum class Comparison {
    at_least,
    exactly,
};

struct EnrichedWord {
    std::string word;
    std::unordered_map<char, int> letter_frequencies;

    EnrichedWord(const std::string& w)
        : word(w)
        , letter_frequencies{}
    {
        for (const auto& c : word) {
            letter_frequencies[c]++;
        }
    }
};

using WordleDictionary = std::vector<EnrichedWord>;

struct WordleInformation {
    static constexpr char absent_char = '_';
    static constexpr char present_char = '.';
    static constexpr char hit_char = '!';

    std::unordered_map<int, char> known_letter_positions;
    std::unordered_map<char, std::pair<Comparison, int>> known_letter_frequencies;

    operator bool() {
        return known_letter_positions.empty() && known_letter_frequencies.empty();
    }

    static WordleInformation from_string(const std::string& word, const std::string& info_string);
};

std::ostream& operator<<(std::ostream& os, const WordleInformation& info);

WordleInformation operator+(const WordleInformation& info1, const WordleInformation& info2);

// convolves two words to extract information
WordleInformation guess_a_word(const EnrichedWord& target, const EnrichedWord& test);

// returns true if the word is possible given the info
bool test_a_word(const EnrichedWord& target, const WordleInformation& test_info);

// reduces the given dictionary based on known information
void dictionary_reduce(WordleDictionary& dict, const WordleInformation& info);

} // namespace wordle-club

#endif // WORDLE_CLUB_COMMON_H