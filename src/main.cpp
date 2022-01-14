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
#include <mutex>
#include <functional>

#include "common.h"

constexpr std::string_view target_word_dictionary = "targets.dict";
constexpr std::string_view guess_word_dictionary = "guesses.dict";

using namespace wordle_club;

std::pair<std::string, int> greedy_guess(
    const WordleDictionary& target_dictionary,
    const WordleDictionary& guess_dictionary,
    const WordleInformation& info
);
void update_information(WordleInformation& info, const std::string& next_guess);

int main() {
    WordleDictionary target_dictionary;
    WordleDictionary guess_dictionary;

    std::cout << "Opening dictionaries...\n";
    std::ifstream target_dict(target_word_dictionary.data());
    std::ifstream guess_dict(guess_word_dictionary.data());

    if (!target_dict || !guess_dict) {
        return 1;
    }
    std::cout << "...dictionaries opened.\n";

    {
        std::cout << "Reading in target words...\n";
        std::string line;
        while (std::getline(target_dict, line)) {
            target_dictionary.push_back(line);
            guess_dictionary.push_back(line);
        }
        std::cout << "...target words read.\n";

        // int max_guesses = 20;
        std::cout << "Reading in guess words...\n";
        while (std::getline(guess_dict, line)) {
            guess_dictionary.push_back(line);
        }
        std::cout << "...guess words read.\n";
    }

    WordleInformation info;
    std::cout << "Assuming first best guess is roate...\n";
    update_information(info, "roate");

    std::cout << "total info: " << info << '\n';

    std::cout << "Reducing target dictionary based on updated info...\n";
    dictionary_reduce(target_dictionary, info);
    std::cout << "...dictionary reduced to size " << target_dictionary.size() << "\n";

    while (target_dictionary.size() > 1) {
        std::cout << "There are " << target_dictionary.size() << " possible words remaining:\n";
        int i = 0;
        for (auto it = target_dictionary.begin(); it != target_dictionary.end(); ++it, ++i) {
            std::cout << it->word << ' ';
            if (i == 12) {
                std::cout << '\n';
                i = 0;
            }
        }
        if (i != 0) {
            std::cout << '\n';
        }

        std::cout << "Determining best greedy guess...\n";
        auto [next_guess, reduction] = greedy_guess(target_dictionary, guess_dictionary, info);
        std::cout << "...best guess is [" << next_guess << "], with reduction value of " << reduction << "\n.";

        update_information(info, next_guess);
        std::cout << "total info: " << info << '\n';

        std::cout << "Reducing target dictionary based on updated info...\n";
        dictionary_reduce(target_dictionary, info);
        std::cout << "...dictionary reduced to size " << target_dictionary.size() << "\n";
    }

    if (target_dictionary.size() == 1) {
        std::cout << "The wordle is: " << target_dictionary[0].word << '\n';
    }
    else {
        std::cout << "No viable wordle found\n";
    }
}

std::pair<std::string, int> greedy_guess(
    const WordleDictionary& target_dictionary,
    const WordleDictionary& guess_dictionary,
    const WordleInformation& info
)
{
    std::unordered_map<std::string, int> guess_scores;
    for (const auto& guess : guess_dictionary) {
        guess_scores[guess.word] = 0;
    }

    // For each target word...
    for (const auto& target : target_dictionary) {
        std::cout << '.';
        // std::cout << "Running all guesses against target word [" << target.word << "]...\n";

        std::mutex m;
        auto it = guess_dictionary.begin();

        std::function<void()> check_routine = [&](){
            {
                while (true) {
                    wordle_club::WordleDictionary::const_iterator guess_it;

                    {
                        std::unique_lock<std::mutex> lock(m);
                        if (it == guess_dictionary.end()) {
                            return;
                        }
                        guess_it = it++;
                    }

                    auto info = guess_a_word(target, *guess_it);

                    for (const auto& t : target_dictionary) {
                        if (!test_a_word(t, info)) {
                            guess_scores[guess_it->word]++;
                        }
                    }
                }
            }
        };

        std::vector<std::thread> threads;
        for (int i = 0; i < 8; ++i) {
            threads.emplace_back(check_routine);
        }

        for (auto& t : threads) {
            t.join();
        }
    }

    auto max_score_guess = std::max_element(guess_scores.begin(), guess_scores.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.second < rhs.second;
    });

    // std::cout << "Sorting results by best-first-guess...\n";
    // std::sort(guess_words.begin(), guess_words.end(), [](const auto& lhs, const auto& rhs) {
    //     return lhs.second < rhs.second;
    // });
    // std::cout << "...results sorted.\n";

    // std::cout << "Printing results...\n";
    // for (const auto& guess : guess_words) {
    //     std::cout << guess.first.word << " eliminates on average " << static_cast<double>(guess.second) / target_words.size() << '\n';
    // }

    return *max_score_guess;
}

void update_information(WordleInformation& info, const std::string& guess) {
    std::string info_string;

    bool done = false;
    while (!done) {
        try {
            std::cout << "Enter info string (._!): ";
            std::cin >> info_string;
            auto new_info = WordleInformation::from_string(guess, info_string);
            info = info + new_info;
            done = true;
        }
        catch(const std::exception& e) {
            std::cout << e.what() << '\n';
        }
    }
}