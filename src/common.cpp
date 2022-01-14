#include <common.h>

#include <algorithm>
#include <iostream>

namespace wordle_club {

WordleInformation WordleInformation::from_string(const std::string& word, const std::string& info_string) {
    if (info_string.size() != word.size()) {
        throw std::runtime_error("string size mismatch");
    }

    if (!std::all_of(info_string.begin(), info_string.end(), [=](const auto& c) {
        return c == absent_char || c == present_char || c == hit_char;
    })) {
        throw std::runtime_error("must contain only '_', '.', '!'");
    }

    WordleInformation info;

    for (int i = 0; i < info_string.size(); ++i) {
        auto info_char = info_string[i];
        auto word_char = word[i];

        switch(info_char) {
            case absent_char:
                if (info.known_letter_frequencies.count(word_char)) {
                    auto freq = info.known_letter_frequencies[word_char].second;
                    info.known_letter_frequencies[word_char] = { Comparison::exactly, freq };
                }
                else {
                    info.known_letter_frequencies[word_char] = { Comparison::exactly, 0 };
                }
                break;
            case hit_char:
                info.known_letter_positions[i] = word_char;
                [[fallthrough]];
            case present_char:
                if (info.known_letter_frequencies.count(word_char)) {
                    info.known_letter_frequencies[word_char].second++;
                }
                else {
                    info.known_letter_frequencies[word_char] = { Comparison::at_least, 1 };
                }
                break;
        }
    }

    return info;
}

WordleInformation operator+(const WordleInformation& info1, const WordleInformation& info2) {
    WordleInformation info = info1;

    for (const auto& [c, f] : info2.known_letter_frequencies) {
        (void)info.known_letter_frequencies[c];

        info.known_letter_frequencies[c] = std::max(info.known_letter_frequencies[c], info2.known_letter_frequencies.at(c));
    }

    for (const auto& [p, c] : info2.known_letter_positions) {
        info.known_letter_positions[p] = c;
    }

    return info;
}

std::ostream& operator<<(std::ostream& os, const WordleInformation& info) {
    for (int i = 0; i < 5; i++) {
        if (info.known_letter_positions.count(i)) {
            os << info.known_letter_positions.at(i);
        }
        else {
            os << "_";
        }
    }

    os << " ";
    for (const auto& [c, f] : info.known_letter_frequencies) {
        os << '[' << c;
        switch(f.first) {
            case Comparison::at_least:
                os << ">=";
                break;
            case Comparison::exactly:
                os << "==";
                break;
        }
        os << f.second;

        os << ']';
    }

    return os;
}

// convolves two words to extract information
WordleInformation guess_a_word(const EnrichedWord& target, const EnrichedWord& test) {
    WordleInformation info;

    for (int i = 0; i < target.word.size(); ++i) {
        if (target.word[i] == test.word[i]) {
            info.known_letter_positions[i] = target.word[i];
        }
    }

    for (const auto& [c, f] : test.letter_frequencies) {
        // f is 1 or greater always at this point
        if (target.letter_frequencies.count(c) == 0) {
            info.known_letter_frequencies[c] = { Comparison::exactly, 0 };
        }
        else if (f <= target.letter_frequencies.at(c)) {
            info.known_letter_frequencies[c] = { Comparison::at_least, f };
        }
        else {
            info.known_letter_frequencies[c] = { Comparison::exactly, target.letter_frequencies.at(c) };
        }
    }

    return info;
}

// returns true if the word is possible given the info
bool test_a_word(const EnrichedWord& target, const WordleInformation& test_info) {
    for (const auto& [pos, c] : test_info.known_letter_positions) {
        if (target.word[pos] != c) {
            // std::cout << "position mismatch: " << pos << "!=" << c;
            return false;
        }
    }

    for (const auto& [c, relationship] : test_info.known_letter_frequencies) {
        const auto& [comparison, frequency] = relationship;

        int target_frequency = 0;
        if (target.letter_frequencies.count(c) != 0) {
            target_frequency = target.letter_frequencies.at(c);
        }

        switch (comparison) {
        case Comparison::at_least:
            if (target_frequency < frequency) {
                // std::cout << "Frequency mismatch: " << c << "<" << frequency;
                return false;
            }
            break;
        case Comparison::exactly:
            if (target_frequency != frequency) {
                // std::cout << "Frequency mismatch: " << c << "!=" << frequency;
                return false;
            }
            break;
        }
    }

    return true;
}

void dictionary_reduce(WordleDictionary& dict, const WordleInformation& info) {
    dict.erase(
        std::remove_if(dict.begin(), dict.end(), [&](const EnrichedWord& word){
            return !test_a_word(word, info);
        }), dict.end()
    );
}

} // namespace wordle_club