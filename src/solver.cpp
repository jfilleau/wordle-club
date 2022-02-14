#include <wordle/solver.hpp>

#include <algorithm>
#include <string>
#include <sstream>
#include <map>

#include <iostream>

namespace wordle {

class FeedbackGenerator {
public:
    using fb = GuessFeedback;

    FeedbackArray operator()() {
        FeedbackArray ret = feedback_;

        _advance();

        return ret;
    }

private:
    FeedbackArray feedback_{fb::black, fb::black, fb::black, fb::black, fb::black};

    void _advance() {
        for (auto& f : feedback_) {
            switch(f) {
                case fb::black:
                    f = fb::yellow;
                    break;
                case fb::yellow:
                    f = fb::green;
                    break;
                case fb::green:
                    f = fb::black;
                    break;
            }

            // only advance to the next element if we've "rolled over"
            if (f != fb::black) {
                break;
            }
        }
    }
};

static void solve_worst_case(
    const std::vector<Wordle>& wordles,
    const std::vector<Wordle>& guesses,
    std::vector<RankedGuess>& ordered_guesses,
    Solver::ProgressCallback cb = nullptr
)
{
    ordered_guesses.clear();
    ordered_guesses.reserve(wordles.size() + guesses.size());

    int total_guesses = wordles.size() + guesses.size();
    int progress = 0;

    // for each possible guess,
    for (const auto& guess : wordles) {
        if (cb) {
            cb(total_guesses, progress++);
        }
        // apply each possible filter
        FeedbackGenerator gen;

        std::map<Filter, int> bins;
        for (int i = 0; i < 243; ++i) {
            auto feedback = gen();
            Filter filter(guess, feedback);

            // check how many words would be in this bin
            for (const auto& wordle : wordles) {
                if (filter.contains(wordle)) {
                    bins[filter]++;
                }
            }
        }

        RankedGuess this_guess { guess };
        this_guess.is_possible_wordle = true;
        this_guess.score = std::min_element(
            bins.begin(),
            bins.end(),
            [](std::pair<Filter, int> lhs, std::pair<Filter, int> rhs) {
                return lhs.second > rhs.second;
            })->second;

        ordered_guesses.push_back(this_guess);
    }

    for (const auto& guess : guesses) {
        if (cb) {
            cb(total_guesses, progress++);
        }
        // apply each possible filter
        FeedbackGenerator gen;

        std::map<Filter, int> bins;
        for (int i = 0; i < 243; ++i) {
            auto feedback = gen();
            Filter filter(guess, feedback);

            // check how many words would be in this bin
            for (const auto& wordle : wordles) {
                if (filter.contains(wordle)) {
                    bins[filter]++;
                }
            }
        }

        RankedGuess this_guess { guess };
        this_guess.is_possible_wordle = false;
        this_guess.score = std::min_element(
            bins.begin(),
            bins.end(),
            [](std::pair<Filter, int> lhs, std::pair<Filter, int> rhs) {
                return lhs.second > rhs.second;
            })->second;

        ordered_guesses.push_back(this_guess);
    }

    std::sort(ordered_guesses.begin(), ordered_guesses.end(), [](const RankedGuess& lhs, const RankedGuess& rhs){
        return std::tie(lhs.score, lhs.guess) < std::tie(rhs.score, rhs.guess);
    });

    if (cb) {
        cb(total_guesses, progress++);
    }
}

Solver::Solver(std::vector<Wordle>&& wordles, std::vector<Wordle>&& guesses, Strategy strategy) noexcept
    : strategy_(strategy)
    , wordles_(wordles)
    , guesses_(guesses)
{ }

void Solver::solve() {
    switch(strategy_) {
        case Strategy::worst_case:
            solve_worst_case(wordles_, guesses_, ranked_guesses_, progress_callback_);
            break;
        default:
            throw std::runtime_error("Strategy not yet implemented");
    }
}

void Solver::apply_guess(const Wordle& guess, const FeedbackArray& feedback) {
    Filter filter(guess, feedback);

    std::cout << guess << " + " << feedback << " =\n";
    std::cout << filter << '\n';

    wordles_.erase(std::remove_if(wordles_.begin(), wordles_.end(), [&](const Wordle& wordle){
        return ( ! filter.contains(wordle) );
    }), wordles_.end());

    std::cout << "Wordle space reduced to: " << wordles_.size() << '\n';
}

std::vector<RankedGuess> Solver::best_guesses(std::size_t num) const {
    return std::vector<RankedGuess>(
        ranked_guesses_.begin(),
        ranked_guesses_.begin() + std::min(num, ranked_guesses_.size())
    );
}

std::vector<RankedGuess> Solver::best_wordles(std::size_t num) const {
    std::vector<RankedGuess> ranked_guesses;
    auto it = ranked_guesses_.begin();

    for (auto it = ranked_guesses_.begin(); ranked_guesses.size() < num; ++it) {
        while (it != ranked_guesses_.end() && (! it->is_possible_wordle )) {
            ++it;
        }

        if (it == ranked_guesses_.end()) {
            break;
        }

        ranked_guesses.push_back(*it);
    }

    return ranked_guesses;
}

} // namespace wordle