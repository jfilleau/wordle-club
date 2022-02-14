#pragma once

#include <array>
#include <ostream>
#include <tuple>

#include "wordle.hpp"

namespace wordle {

enum class GuessFeedback {
    black = 0,
    yellow = 1,
    green = 2,

    not_present = 0,
    present_but_wrong_spot = 1,
    present_and_correct_spot = 2,
};

using FeedbackArray = std::array<GuessFeedback, 5>;
std::ostream& operator<<(std::ostream& os, const FeedbackArray& fb);

inline FeedbackArray to_feedback(const std::string_view& str) {
    FeedbackArray feedback;

    auto feedback_it = feedback.begin();
    auto str_it = str.begin();

    for (; feedback_it != feedback.end(); ++feedback_it, ++str_it) {
        switch (*str_it) {
            case 'B':
                *feedback_it = GuessFeedback::black;
                break;
            case 'Y':
                *feedback_it = GuessFeedback::yellow;
                break;
            case 'G':
                *feedback_it = GuessFeedback::green;
                break;
        }
    }
    
    return feedback;
}

struct Filter {
public:
    Filter(const Wordle& wordle, const FeedbackArray& feedback) noexcept;

    bool contains(const Wordle& wordle) const;

    bool operator<(const Filter& other) const;
    bool operator==(const Filter& other) const;
    bool operator!=(const Filter& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Filter& filter);

private:
    auto _member_tuple() const {
        return std::tie(possible_chars_, min_chars_, min_is_max_);
    }

private:
    static constexpr std::array<bool, 26> none_set{
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    static constexpr std::array<int, 26> all_min{
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    std::array<std::array<bool, 26>, 5> possible_chars_{
        none_set, none_set, none_set, none_set, none_set
    };
    std::array<int, 26> min_chars_{all_min};
    std::array<bool, 26> min_is_max_{none_set};
};

inline bool Filter::operator<(const Filter& other) const {
    return _member_tuple() < other._member_tuple();
}

inline bool Filter::operator==(const Filter& other) const {
    return _member_tuple() == other._member_tuple();
}

inline bool Filter::operator!=(const Filter& other) const {
    return _member_tuple() != other._member_tuple();
}

} // namespace wordle