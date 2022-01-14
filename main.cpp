// Given a dictionary of randomly selectable words, and
// a dictionary of valid guess words (beyond the randomly selectable words)
// Find the best guess to eliminate the most possible randomly selectables

#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <utility>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

constexpr std::string_view target_word_dictionary = "targets.dict";
constexpr std::string_view guess_word_dictionary = "guesses.dict";

// Two words are tested against each other, which produces information about the word
// That information pares down the list
// The test word that, on average, pares down the list the most is the best first guess

enum Comparison {
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

struct WordInformation {
    std::unordered_map<int, char> known_letter_positions;
    std::unordered_map<char, std::pair<Comparison, int>> known_letter_frequencies;
};

// convolves two words to extract information
WordInformation guess_a_word(const EnrichedWord& target, const EnrichedWord& test) {
    WordInformation info;

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
bool test_a_word(const EnrichedWord& target, const WordInformation& test_info) {
    for (const auto& [pos, c] : test_info.known_letter_positions) {
        if (target.word[pos] != c) {
            // std::cout << "Known position mismatch: ";
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
                // std::cout << "Frequency mismatch: ";
                return false;
            }
            break;
        case Comparison::exactly:
            if (target_frequency != frequency) {
                // std::cout << "Frequency mismatch: ";
                return false;
            }
            break;
        }
    }

    return true;
}


int main() {
    std::cout << "Opening dictionaries...\n";
    std::ifstream target_dict(target_word_dictionary.data());
    std::ifstream guess_dict(guess_word_dictionary.data());

    if (!target_dict || !guess_dict) {
        return 1;
    }
    std::cout << "...dictionaries opened.\n";

    std::vector<EnrichedWord> target_words;

    // word and elimination score
    std::vector<std::pair<EnrichedWord, int>> guess_words;

    {
        // int max_targets = 10;
        std::cout << "Reading in target words...\n";
        std::string line;
        while (std::getline(target_dict, line)) {
            target_words.push_back(line);
            guess_words.push_back({ line, 0 });
            /*
            if (--max_targets == 0) {
                break;
            }
            */
        }
        std::cout << "...target words read.\n";

        // int max_guesses = 20;
        std::cout << "Reading in guess words...\n";
        while (std::getline(guess_dict, line)) {
            guess_words.push_back({ line, 0 });
            /*
            if (--max_guesses == 0) {
                break;
            }
            */
        }
        std::cout << "...guess words read.\n";
    }

    // For each target word...
    for (const auto& target : target_words) {
        std::cout << "Running all guesses against target word [" << target.word << "]...\n";
        for (auto& [guess, score] : guess_words) {
            auto info = guess_a_word(target, guess);
            /*
            std::cout << "Guessed ";

            auto freqs = info.known_letter_frequencies;

            for (int i = 0; i < 5; ++i) {
                if (info.known_letter_positions.count(i) != 0) {
                    std::cout << "[" << info.known_letter_positions[i] << "]";
                    freqs[info.known_letter_positions[i]].second--;
                }
                else if (freqs[guess.word[i]].second) {
                    std::cout << "(" << guess.word[i] << ")";
                    freqs[guess.word[i]].second--;
                }
                else {
                    std::cout << " " << guess.word[i] << " ";
                }
            }

            std::cout << "\n";

            std::cout << "Known positions: ";
            for (const auto& [index, c] : info.known_letter_positions) {
                std::cout << index << ":" << c << " ";
            }
            std::cout << '\n';

            std::cout << "Known frequencies: ";
            for (const auto& [c, freq] : info.known_letter_frequencies) {
                std::cout << c;
                switch (freq.first) {
                case Comparison::at_least:
                    std::cout << ">=";
                    break;
                case Comparison::exactly:
                    // std::cout << "==";
                    break;
                }
                std::cout << freq.second << " ";
            }
            std::cout << '\n';
            */

            for (const auto& t : target_words) {
                if (test_a_word(t, info)) {
                    // std::cout << t.word << " is possible\n";
                }
                else {
                    // std::cout << t.word << " is eliminated\n";
                    score++;
                }
            }
        }
    }

    std::cout << "Sorting results by best-first-guess...\n";
    std::sort(guess_words.begin(), guess_words.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.second < rhs.second;
    });
    std::cout << "...results sorted.\n";

    std::cout << "Printing results...\n";
    for (const auto& guess : guess_words) {
        std::cout << guess.first.word << " eliminates on average " << static_cast<double>(guess.second) / target_words.size() << '\n';
    }
}