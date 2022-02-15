/**
 * (yet another) Wordle Solver
 * John Filleau, 2022
 *
 * Command Line Arguments:
 * --wordles, -w: file that contains new-line separated wordles (possible answers)
 * --guesses,  -g: file that contains new-line separated guesses (can be used as guesses, but will never be answers)
 * --strategy,    -s: strategy used to score guesses, one of
 *      bestcase: scores guesses based on their best possible bucket
 *      worstcase: scores guesses based on their worst possible bucket
 *      mean: scores guesses based on the mean of their buckets
 *      smr: scores words based on the square-mean-root of their buckets (like mean, but favors smaller buckets)
**/

#include <iostream>
#include <fstream>
#include <string_view>
#include <vector>
#include <iomanip>

#include <argparse/argparse.hpp>

#include <wordle/solver.hpp>

enum WordleValidity {
    valid,
    incorrect_length,
    non_alpha_char,
};

std::vector<wordle::Wordle> read_file(std::ifstream& file);
argparse::ArgumentParser parse_args(int argc, const char* const argv[]);
WordleValidity check_wordle_validity(const std::string& word);
bool check_feedback_validity(const std::string& feedback);
void play_game(wordle::Solver& solver, const argparse::ArgumentParser& parser);
void run_analytics(const std::vector<wordle::Wordle>& wordles, wordle::Solver& solver, const argparse::ArgumentParser& parser);

int main(int argc, const char* const argv[]) {
    auto parser = parse_args(argc, argv);

    std::ifstream wordle_file(parser.get<const char*>("--wordles"));
    if (!wordle_file) {
        std::cerr << "Unable to open wordle file: " << parser.get<const char*>("--wordles") << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    std::cout << "Reading wordles file...\n";
    std::vector<wordle::Wordle> wordles = read_file(wordle_file);
    std::cout << "...read " << wordles.size() << " words\n";

    std::ifstream guess_file(parser.get<const char*>("--guesses"));
    if (!guess_file) {
        std::cerr << "Unable to open guess file: " << parser.get<const char*>("--guesses") << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    std::cout << "Reading guesses file...\n";
    std::vector<wordle::Wordle> guesses = read_file(guess_file);
    std::cout << "...read " << guesses.size() << " words\n";

    std::vector<wordle::Wordle> all_wordles = wordles;

    wordle::Solver solver(std::move(wordles), std::move(guesses));
    solver.on_progress([](int total, int progress) {
        std::cout << '\r' << progress << '/' << total;
    });
}

std::vector<wordle::Wordle> read_file(std::ifstream& file) {
    std::vector<wordle::Wordle> wordles;

    std::string line;
    int line_number = 0;
    while (++line_number, std::getline(file, line)) {
        if (auto validity = check_wordle_validity(line); validity != WordleValidity::valid) {
            std::stringstream ss;
            ss << "bad input on line " << line_number << "; ";
            switch (validity) {
                case WordleValidity::incorrect_length:
                    ss << "incorrect length";
                    break;
                case WordleValidity::non_alpha_char:
                    ss << "not alpha/lower character";
                    break;
            }
            throw std::runtime_error(ss.str());
        }

        wordles.emplace_back(line);
    }

    return wordles;
}

argparse::ArgumentParser parse_args(int argc, const char* const argv[]) {
    argparse::ArgumentParser parser("Wordle Solver", "0.0.1");
    parser.add_argument("--wordles", "-w")
        .nargs(1)
        .default_value("wordles.txt")
        .help("file that contains new-line separated wordles (possible answers)");
    parser.add_argument("--guesses", "-g")
        .nargs(1)
        .default_value("guesses.txt")
        .help("file that contains new-line separated guesses (can be used as guesses, but will never be answers)");
    parser.add_argument("--strategy", "-s")
        .nargs(1)
        .default_value("worstcase")
        .help("The selection strategy to use, one of <worstcase, bestcase, mean, sms>")
        .action([&](const std::string& value) {
            static const std::vector<std::string> choices = { "worstcase", "bestcase", "mean", "smr" };
            if (std::find(choices.begin(), choices.end(), value) != choices.end()) {
                return value;
            }
            std::cerr << parser;
            std::exit(1);
        });
    parser.add_argument("--firstguess", "-f")
        .nargs(1)
        .help("specifies an initial first guess (skips the first solving step)")
        .action([&](const std::string& value) {
            if ( check_wordle_validity(value) ) {
                return;
            }
            std::cerr << parser;
            std::exit(1);
        });
    parser.add_argument("--analytics", "-a")
        .implicit_value(true)
        .default_value(false)
        .help("calculate metrics about a given method");

    try {
        parser.parse_args(argc, argv);
    }
        catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    return parser;
}

WordleValidity check_wordle_validity(const std::string& word) {
    if (word.size() != 5) {
        return WordleValidity::incorrect_length;
    }
    for (const auto& c : word) {
        if ( !std::islower(c) || !std::isalpha(c) ) {
            return WordleValidity::non_alpha_char;
        }
    }

    return WordleValidity::valid;
}

bool check_feedback_validity(const std::string& feedback) {
    if (feedback.size() != 5) {
        return false;
    }
    static constexpr std::string_view valid_chars = "BYG";
    if (feedback.find_first_not_of(valid_chars) != std::string::npos) {
        return false;
    }

    return true;
}

void play_game(wordle::Solver& solver, const argparse::ArgumentParser& parser) {
    // wordle::Wordle
    std::string guess;

    if ( ! parser.is_used("--firstguess") ) {
        std::cout << "Solving first iteration...\n";
        solver.solve();
        std::cout << "\n\n";
        auto best_guesses = solver.best_guesses(20);
        std::cout << "Best guesses:\n";
        for (const auto& best : best_guesses) {
            std::cout << (best.is_possible_wordle ? '*' : ' ') << std::setw(6) << best.guess << std::setw(6) << best.score << '\n';
        }

        do {
            std::cout << "Enter guess: ";
            std::getline(std::cin, guess);
        } while (check_wordle_validity(guess) != WordleValidity::valid);
    }
    else {
        guess = parser.get<std::string>("--firstguess");
    }

    std::cout << "Using first guess: " << guess << '\n';

    while (solver.wordles().size() > 1) {

        std::string feedback;
        do {
            std::cout << "Enter feedback: ";
            std::getline(std::cin, feedback);
        } while ( ! check_feedback_validity(feedback));

        wordle::Wordle w_guess(guess);
        wordle::FeedbackArray w_feedback = wordle::to_feedback(feedback);

        std::cout << "Reducing search space...\n";
        solver.apply_guess(w_guess, w_feedback);
        std::cout << "...search space reduced.\n";

        std::cout << "Solving...\n";
        solver.solve();
        std::cout << "\n\n";

        auto best_guesses = solver.best_guesses(20);
        std::cout << "Best guesses:\n";
        for (const auto& best : best_guesses) {
            std::cout << (best.is_possible_wordle ? '*' : ' ') << std::setw(6) << best.guess << std::setw(6) << best.score << '\n';
        }
        best_guesses = solver.best_wordles(20);
        std::cout << "Best wordles:\n";
        for (const auto& best : best_guesses) {
            std::cout << (best.is_possible_wordle ? '*' : ' ') << std::setw(6) << best.guess << std::setw(6) << best.score << '\n';
        }

        std::string guess;
        do {
            std::cout << "Enter guess: ";
            std::getline(std::cin, guess);
        } while (check_wordle_validity(guess) != WordleValidity::valid);
    }

    std::string dummy;
    std::getline(std::cin, dummy);
}

void run_analytics(const std::vector<wordle::Wordle>& wordles, wordle::Solver& solver, const argparse::ArgumentParser& parser) {
    std::map<wordle::Wordle, int> required_guesses;

    int wordle_number = 1;
    int total_wordles = wordles.size();
    for (const auto& w : wordles) {
        std::cout << '[' << wordle_number++ << "/" << total_wordles << "] Finding strategy if wordle is: " << w << '\n';
        int steps = 0;

        wordle::Wordle guess("");
        if ( ! parser.is_used("--firstguess") ) {
            solver.solve();
            std::cout << "\n";
            auto guess = solver.best_guesses(1)[1].guess;
        }
        else {
            guess = parser.get<std::string_view>("--firstguess");
        }

        while (solver.wordles().size() > 1) {
            std::string feedback;
            do {
                std::cout << "Enter feedback: ";
                std::getline(std::cin, feedback);
            } while ( ! check_feedback_validity(feedback));

            wordle::Wordle w_guess(guess);
            wordle::FeedbackArray w_feedback = wordle::to_feedback(feedback);

            std::cout << "Reducing search space...\n";
            solver.apply_guess(w_guess, w_feedback);
            std::cout << "...search space reduced.\n";

            std::cout << "Solving...\n";
            solver.solve();
            std::cout << "\n\n";

            auto best_guesses = solver.best_guesses(20);
            std::cout << "Best guesses:\n";
            for (const auto& best : best_guesses) {
                std::cout << (best.is_possible_wordle ? '*' : ' ') << std::setw(6) << best.guess << std::setw(6) << best.score << '\n';
            }
            best_guesses = solver.best_wordles(20);
            std::cout << "Best wordles:\n";
            for (const auto& best : best_guesses) {
                std::cout << (best.is_possible_wordle ? '*' : ' ') << std::setw(6) << best.guess << std::setw(6) << best.score << '\n';
            }

            std::string guess;
            do {
                std::cout << "Enter guess: ";
                std::getline(std::cin, guess);
            } while (check_wordle_validity(guess) != WordleValidity::valid);
        }
    }



    std::string dummy;
    std::getline(std::cin, dummy);
}