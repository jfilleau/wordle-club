#pragma once

#include <cstddef>
#include <functional>
#include <future>
#include <istream>
#include <vector>

#include "filter.hpp"
#include "wordle.hpp"

namespace wordle {

enum class Strategy {
    worst_case,
    best_case,
    mean,
    square_mean_root,
};

struct RankedGuess {
    Wordle guess;
    bool is_possible_wordle;
    double score;
};

class Solver {
public:
    using ProgressCallback = std::function<void(int /* total_evaluations */, int /* completed_evaluations */)>;

    Solver(std::vector<Wordle>&& wordles, std::vector<Wordle>&& guesses,
        Strategy strategy = Strategy::worst_case) noexcept;

    void solve();

    void apply_guess(const Wordle& word, const FeedbackArray& feedback);

    std::vector<RankedGuess> best_guesses(std::size_t num) const;
    std::vector<RankedGuess> best_wordles(std::size_t num) const;

    void on_progress(const ProgressCallback& cb) {
        progress_callback_ = cb;
    }

    const std::vector<Wordle>& wordles() const {
        return wordles_;
    }

    const std::vector<Wordle>& guesses() const {
        return guesses_;
    }

private:
    Strategy strategy_{Strategy::worst_case};
    std::vector<Wordle> wordles_{};
    std::vector<Wordle> guesses_{};

    std::vector<RankedGuess> ranked_guesses_{};

    ProgressCallback progress_callback_{};
};

} // namespace wordle