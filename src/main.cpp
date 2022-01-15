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
#include <iomanip>

#include "common.h"

constexpr std::string_view target_word_dictionary = "targets.dict";
constexpr std::string_view guess_word_dictionary = "guesses.dict";

using namespace wordle_club;

std::vector<std::pair<std::string, double>> greedy_guess(
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
    /*
    std::cout << "Assuming first best guess is roate...\n";
    update_information(info, "roate");

    std::cout << "total info: " << info << '\n';

    std::cout << "Reducing target dictionary based on updated info...\n";
    dictionary_reduce(target_dictionary, info);
    std::cout << "...dictionary reduced to size " << target_dictionary.size() << "\n";
    */

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
        auto top_ten_guesses = greedy_guess(target_dictionary, guess_dictionary, info);
        std::cout << "...best guesses are:\n";
        {
            int line = 0;
            for (const auto& [g, v] : top_ten_guesses) {
                std::cout << "[" << g << ":" << std::fixed << std::setprecision(3) << v << "]";
                if (++line == 5) {
                    line = 0;
                    std::cout << '\n';
                }
            }
            if (line != 0) {
                std::cout << '\n';
            }
        }

        std::string next_guess;
        do {
            std::cout << "Select guess: ";
            std::getline(std::cin, next_guess);
        } while (std::find_if(top_ten_guesses.begin(), top_ten_guesses.end(), [&](const auto& element) {
            return element.first == next_guess;
        }) == top_ten_guesses.end());


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

    std::string line;
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, line);
}

std::vector<std::pair<std::string, double>> greedy_guess(
    const WordleDictionary& target_dictionary,
    const WordleDictionary& guess_dictionary,
    const WordleInformation& info
)
{
    std::unordered_map<std::string, double> guess_scores;
    for (const auto& guess : guess_dictionary) {
        guess_scores[guess.word] = 0.0;
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
                    int this_guess_score = 0;

                    for (const auto& t : target_dictionary) {
                        if (!test_a_word(t, info)) {
                            // guess_scores[guess_it->word]++;
                            this_guess_score++;
                        }
                    }

                    guess_scores[guess_it->word] += std::sqrt(this_guess_score);
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

    std::vector<std::pair<std::string, double>> top_ten_guesses;
    top_ten_guesses.reserve(guess_scores.size());
    std::move(guess_scores.begin(), guess_scores.end(), std::back_inserter(top_ten_guesses));

    std::sort(top_ten_guesses.begin(), top_ten_guesses.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.second < rhs.second;
    });

    if (top_ten_guesses.size() > 10) {
        top_ten_guesses.erase(top_ten_guesses.begin(), top_ten_guesses.begin() + top_ten_guesses.size() - 10);
    }

    // std::cout << "Sorting results by best-first-guess...\n";
    // std::sort(guess_words.begin(), guess_words.end(), [](const auto& lhs, const auto& rhs) {
    //     return lhs.second < rhs.second;
    // });
    // std::cout << "...results sorted.\n";

    // std::cout << "Printing results...\n";
    // for (const auto& guess : guess_words) {
    //     std::cout << guess.first.word << " eliminates on average " << static_cast<double>(guess.second) / target_words.size() << '\n';
    // }

    return top_ten_guesses;
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